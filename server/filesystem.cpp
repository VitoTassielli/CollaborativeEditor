#include "filesystem.h"

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

    query.prepare("INSERT INTO files(FileId,Filename) VALUES ((:userid), (:filename))");
    query.bindValue(":filename", filename);
    query.bindValue(":userid", id->second);
    if (query.exec()){
        QFile m_file (filename);
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

        QJsonObject object;
        QJsonArray array;
        for(Letter* lett: it->second->getLetter()){
           array.append(lett->toJSon());
        }
        object.insert("letterArray",array);
        object.insert("type", "OPEN");
        object.insert("fileid", fileid);

        if(socket->state() == QAbstractSocket::ConnectedState)
        {
            qDebug() << "Invio file";
            socket->write(IntToArray(QJsonDocument(object).toJson().size())); //write size of data
            if(socket->write(QJsonDocument(object).toJson()) == -1){
                qDebug() << "File failed to send";
                return nullptr;
            } //write the data itself
            socket->waitForBytesWritten();
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
        QFile m_file (QString::number(fileid));
        m_file.open(QFile::ReadOnly);

        QByteArray q = m_file.readAll();

        QJsonDocument document = QJsonDocument::fromJson(q);
        QJsonObject object = document.object();
        QJsonValue value = object.value("letterArray");
        QJsonArray letterArray = value.toArray();
        object.insert("type", "OPEN");
        object.insert("fileid", fileid);

        if(socket->state() == QAbstractSocket::ConnectedState)
        {
            qDebug() << "Invio file";
            socket->write(IntToArray(QJsonDocument(object).toJson().size())); //write size of data
            if(socket->write(QJsonDocument(object).toJson()) == -1){
                qDebug() << "File failed to send";
                return nullptr;
            } //write the data itself
            socket->waitForBytesWritten();
        }
        m_file.close();

        qDebug() << "File sent";

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
        qDebug() << "File saved in the file system";
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

std::map<int, FileHandler*> FileSystem::getFiles() {
    return this->files;
}

void FileSystem::disconnectClient(QTcpSocket* socket){
    files.at(sock_file.at(socket))->removeActiveUser(socket);
}


