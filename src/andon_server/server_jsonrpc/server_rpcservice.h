#ifndef JSONRPCSERVICE_H
#define JSONRPCSERVICE_H

#include "qjsonrpcservice.h"
#include <QThread>
#include <QDebug>

#include "dbwrapper.h"
//_______QtTelnet class_______________
#include "qttelnet.h"

//class QJsonRpcService;
class ServerRpcService : public QJsonRpcService
{
    Q_OBJECT
    Q_CLASSINFO("serviceName", JSONRPC_SERVER_SERVICENAME)
public:
    ServerRpcService(QObject *parent = 0);
    ~ServerRpcService();
private:
    QString curClientIP();

signals:
    void SendSMS(const QString &PhoneNumber, const QString &Sms_text, const QString &Lang_name, const int &SmsLogId,  const int &att);
    void clientDisconnected(const QString &clientIp);

public slots:
    QString SQLQuery2Json(const QString & sqlquery);
//    QString SQLQueryVariant(const QString & sqlquery);
    QString SQLQueryExec(const QString & sqlquery);
    void executeProc(const QString & sqlquery);

    void StopSms(const QString &Sms_id);
    void PauseSms(const QString &Sms_id);
    void ResumeSms(const QString &Sms_id);
    void CancelSms(const QString &Sms_id);
    QString StartSms(const QString &sms_param);
    int telnetDeclareKanban(const QByteArray &kanbanNumber);
    void setDB(DBWrapper* db);
    void setTelnet(QtTelnet* telnetClnt);




private:
    DBWrapper* andondb;
    QtTelnet* telnetClient;
    QMap<QString,QMap<QString,QThread*>> ThreadsMap;

};


#endif //JSONRPCSERVICE_H
