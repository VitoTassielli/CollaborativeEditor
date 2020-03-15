#include "uri.h"
#include "ui_uri.h"

Uri::Uri(Socket *s, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Uri),
    sock(s)
{
    ui->setupUi(this);
    setWindowTitle("Error");
    setWindowModality(Qt::WindowModal);
}

Uri::~Uri()
{
    delete ui;
}

void Uri::on_pushButton_clicked()
{
    this->close();
}
