#include "filesystem.h"
#include <QCryptographicHash>

#define STR_SALT_KEY "qwerty"
#define DATA_SIZE 1024*1024

// class to handle all files in a map

FileSystem *FileSystem::instance = nullptr;

static inline QByteArray IntToArray(qint32 source);

FileSystem* FileSystem::getInstance(){
    if(!instance){
        instance = new FileSystem();
        const QString DRIVER("QSQLITE");
        if(QSqlDatabase::isDriverAvailable(DRIVER)){
            instance->db = QSqlDatabase::addDatabase(DRIVER);
            instance->db.setDatabaseName("user.db");
            if (!instance->db.open())
            {
                 qDebug() << "Error: connection with database fail";
            }
            else
            {
                qDebug() << "Database: connection ok";
            }

        }
    }
    return FileSystem::instance;
}

FileHandler* FileSystem::createFile(QString filename, QTcpSocket *socket){

    auto id = sock_id.find(socket);
    if(id == sock_id.end()) return nullptr;//il socket è autenticato o no

    auto file = sock_file.find(socket);
    if(file != sock_file.end()){
        // disconnessione di un client da un file
        FileHandler *fh = files.at(file->second);
        fh->removeActiveUser(socket);
    }
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM file WHERE username=((:userid), (:filename))");
    query.bindValue(":filename", filename);
    query.bindValue(":userid", id->second);
    int count = -1;
    if (query.exec())
    {
        if (query.next())
        {
            count = query.value(0).toInt();
        }
    }
    else{
        qDebug() << "Query not executed";
    }
    if(count != 0){
        qDebug("This filename is already taken by the user");
    }

    query.prepare("INSERT INTO files(Username,Filename) VALUES ((:userid), (:filename))");
    query.bindValue(":filename", filename);
    query.bindValue(":userid", id->second);
    if (query.exec()){
        QFile m_file (filename); // crea il file col nome id
        m_file.open(QFile::ReadOnly);

        QVector<Letter*> letters;
        int fileid = query.lastInsertId().toInt();
        FileHandler *fh = new FileHandler(std::move(letters), fileid);
        fh->insertActiveUser(socket);

        sock_file.insert(std::pair<QTcpSocket*, int> (socket, fileid)); //associate file to socket
        files.insert(std::pair<int, FileHandler*> (fileid, fh));

        qDebug() << "Insert executed";
        return fh;
    }
    else{
        qDebug() << "Insert not executed";
    }

    return nullptr;
}

FileHandler* FileSystem::sendFile(int fileid, QTcpSocket *socket){
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);

    QFile inFile(QString::number(fileid));
    inFile.open(QFile::ReadOnly);

    //TODO: controllare che il client ha accesso

    if(sock_id.find(socket) == sock_id.end()) return nullptr;//il socket è autenticato o no
    auto file = sock_file.find(socket);

    if(file != sock_file.end()){
        // disconnessione di un client da un file
        FileHandler *fh = files.at(fileid);
        fh->removeActiveUser(socket);
    }
    auto it = files.find(fileid);
    if (it != files.end()){
        // il file è già in memoria principale e può essere mandato
        // serializzarlo
        int size = inFile.size();
        QJsonObject object;
        QJsonArray array;
        for(Letter* lett: it->second->getLetter()){
           array.append(lett->toJSon());
        }
        object.insert("letterArray",array);
        object.insert("type", "OPEN");
        object.insert("fileid", fileid);
        object.insert("size", size);

        int remaining = size;

        while(!inFile.atEnd())
        {
            int chunk = remaining%(DATA_SIZE);
            QByteArray qa = inFile.read(chunk);
            qDebug() << "emitting dataRead()";
            remaining -= chunk;
            emit dataRead(qa, socket, remaining);
        }

        qDebug() << "File sent";
        FileHandler *fh = it->second;
        fh->insertActiveUser(socket);
        sock_file.insert(std::pair<QTcpSocket*, int> (socket, fileid)); //associate file to socket

        return fh;
    }
    else{
        qDebug() << "Inizio l'invio del file";
        // apre il file, lo scrive in un DataStream che poi invierà
        QByteArray buffer_tot;
        int size = (int)inFile.size();
        qDebug() << size;
        QJsonObject file_info;
        file_info.insert("type", "OPEN");
        file_info.insert("fileid", fileid);
        file_info.insert("size", size);
        int remaining = size;
        //manda il file info
        if(socket->state() == QAbstractSocket::ConnectedState){
            qDebug() << "Invio file";
            if(socket->write(QJsonDocument(file_info).toJson()) == -1){
                qDebug() << "File info failed to send";
                return nullptr;
            } //write the data itself
            socket->waitForBytesWritten();
        }
        // manda i chunk
        while(remaining > 0)
        {
            int chunk = remaining % (DATA_SIZE);
            QByteArray qa = inFile.read(chunk);
            buffer_tot.append(qa);
            qDebug() << "emitting dataRead()";
            remaining -= chunk;
            emit dataRead(qa, socket, remaining);
        }
        inFile.close();

        qDebug() << "File sent";

        // lo salva in memoria ram
        QJsonDocument document = QJsonDocument::fromJson(buffer_tot);
        QJsonObject object = document.object();
        QJsonValue value = object.value("letterArray");
        QJsonArray letterArray = value.toArray();

        QVector<Letter*> letters;

        foreach (const QJsonValue& v, letterArray)
        {
            QChar letter = v.toObject().value("letter").toString().at(0);
            QString ID = v.toObject().value("letterID").toString();

            QJsonArray array_tmp = v.toObject().value("position").toArray();
            QVector<int> fractionals;
            for(auto fractional : array_tmp) {
                fractionals.append(fractional.toInt());
            }

            Letter *letter_tmp = new Letter(letter, fractionals, ID);
            letters.append(std::move(letter_tmp));
        }
        FileHandler *fh = new FileHandler(std::move(letters), fileid);
        fh->insertActiveUser(socket);

        files.insert(std::pair<int, FileHandler*> (fileid, fh));
        sock_file.insert(std::pair<QTcpSocket*, int> (socket, fileid)); //associate file to socket
        qDebug() << "File saved in RAM";
        return fh;
    }
}

QByteArray IntToArray(qint32 source) //Use qint32 to ensure that the number have 4 bytes
{
    //Avoid use of cast, this is the Qt way to serialize objects
    QByteArray temp;
    QDataStream data(&temp, QIODevice::ReadWrite);
    data << source;
    return temp;
}

void FileSystem::checkLogin(QString username, QString password, QTcpSocket *socket){

    QSqlQuery query;
    QVector<QString> files;
    QJsonArray files_array;

    query.prepare("SELECT userid FROM password WHERE username = (:username) AND password = (:password)");
    query.bindValue(":username", username);
    //QByteArray saltedPsw = password.append(STR_SALT_KEY).toUtf8();
    //QString encryptedPsw = QString(QCryptographicHash::hash(saltedPsw, QCryptographicHash::Md5));
    //query.bindValue(":password", encryptedPsw);
    query.bindValue(":password", password);
    int id = -1;
    if (query.exec())
    {
        if (query.next())
        {
            id =  query.value("userid").toInt();
        }
    }
    else{
        qDebug() << "Query not executed";
    }
    QJsonArray file_array;
    QJsonObject final_object;
    if(id != -1){
        QSqlQuery query;
        sock_id.insert(std::pair<QTcpSocket*, int> (socket, id)); //associate id to socket
        query.prepare("SELECT filename, rowid FROM files WHERE username = (:username)");
        query.bindValue(":username", username);
        if (query.exec())
        {
            while (query.next())
            {
               QJsonObject item_data;
               QString name = query.value("filename").toString();
               int fileid = query.value("rowid").toInt();
               qDebug() << name;
               item_data.insert("filename", QJsonValue(name));
               item_data.insert("fileid", QJsonValue(fileid));

               file_array.push_back(QJsonValue(item_data));
            }
            final_object.insert(QString("files"), QJsonValue(file_array));
        }
        else{
            qDebug() << "Query not executed";
        }
    }
    final_object.insert("id", QJsonValue(id));

    if(socket->state() == QAbstractSocket::ConnectedState){
        qDebug() << "Risposta al LOGIN:\n" << QJsonDocument(final_object).toJson().data();
        socket->write(QJsonDocument(final_object).toJson());
        socket->waitForBytesWritten(1000);
    }

}

void FileSystem::storeNewUser(QString username, QString psw, QTcpSocket *socket) {
    QSqlQuery sqlQuery;

    // check che username non sia gia' stato preso
    sqlQuery.prepare("SELECT COUNT(*) FROM PASSWORD WHERE username=(:username)");
    sqlQuery.bindValue(":username", username);
    int count = -1;
    if (sqlQuery.exec())
    {
        if (sqlQuery.next())
        {
            count = sqlQuery.value(0).toInt();
        }
    }
    else{
        qDebug() << "Query not executed";
        // EMIT SEGNALE PROBLEMA SERVER FAILED TO RESPOND
        emit signUpResponse("SERVER_FAILURE", false, socket);
        return;
    }
    if(count > 0){
        qDebug("Username already taken");
        // EMIT SEGNALE CAMBIA USERNAME
        emit signUpResponse("INVALID_USERNAME", false, socket);
        return;
    }

    int userID = count + 1;     // last USERID used = count + 1
    // Encrypt password (sale + md5 hash)
    QByteArray saltedPsw = psw.append(STR_SALT_KEY).toUtf8();
    QString encryptedPsw = QString(QCryptographicHash::hash(saltedPsw, QCryptographicHash::Md5));

    /* Insert new user in DB */
    sqlQuery.prepare("INSERT INTO PASSWORD(userid, username, password) VALUES ((:userID),(:username),(:password))");    // safe for SQL injection
    sqlQuery.bindValue(":userID", userID);
    sqlQuery.bindValue(":username", username);
    sqlQuery.bindValue(":password", encryptedPsw);
    //sqlQuery.bindValue(":password", psw);

    if (sqlQuery.exec()){
        // EMIT SIGN UP SUCCESSFUL
        emit signUpResponse("SUCCESS", true, socket);
    } else {
        qDebug() << "INSERT new user not executed!";
        // EMIT segnale server failed to respond
        emit signUpResponse("SERVER_FAILURE", false, socket);
        return;
    }
}

std::map<int, FileHandler*> FileSystem::getFiles() {
    return this->files;
}

void FileSystem::disconnectClient(QTcpSocket* socket){
    files.at(sock_file.at(socket))->removeActiveUser(socket);
}


