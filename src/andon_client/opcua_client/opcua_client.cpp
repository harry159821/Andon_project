#include "opcua_client.h"

#include <QtOpcUa/QOpcUaClient>
#include <QtOpcUa/QOpcUaMonitoredValue>
#include <QtOpcUa/QOpcUaMonitoredEvent>
#include <QtOpcUa/QOpcUaSubscription>
#include <QtOpcUa/QOpcUaNode>
#include <QtOpcUa/QOpcUaProvider>
#include <QtCore/QDebug>
#include <QtCore/QScopedPointer>


//#include <QTimer>
#include <QTcpServer>
#include <QTcpSocket>
#include <QHostAddress>
//#include <QApplication>




const QByteArray constBAVariableName = "VARIABLE_NAME";
const QByteArray constBAVariableId = "VARIABLE_ID";
const QByteArray constDevId = "TCPDEVICE_ID";
const QString constBAinputCode("inputCode");
const QString constBAloaded("loaded");

OpcUaClient::OpcUaClient(QObject *parent)
    : QObject(parent)
    , m_pClient(0)
    , m_pProvider(new QOpcUaProvider(this))
    , m_subscription(0)
    , m_monitorTimer(new QTimer)
    , m_triggerNode(0)
{

    m_pClient = m_pProvider->createClient("freeopcua");

    if (!m_pClient) {
        qWarning() << "Could not initialize QtOpcUa plugin: freeopcua";
        exit(EXIT_FAILURE);
    }

    QObject::connect(this, &OpcUaClient::serverFound,[this](){
        qDebug()<<1;
        if(!m_pClient){
            qDebug()<<"Error - !m_pClient";
            m_pClient = m_pProvider->createClient("freeopcua");
        }
        qDebug()<<2;
        bool res = m_pClient->connectToEndpoint(QString("opc.tcp://%1:%2").arg(OPCUA_IP).arg(OPCUA_PORT));
        qDebug()<<6;
        if (!res) {
            qWarning() << QString("Failed to connect to endpoint: opc.tcp://%1:%2").arg(OPCUA_IP).arg(OPCUA_PORT);
            exit(EXIT_FAILURE);
        }
        qDebug()<<7;
    });

    QObject::connect(m_monitorTimer, &QTimer::timeout,this, &OpcUaClient::subscribeObjects);

    QObject::connect(m_pClient, &QOpcUaClient::connectedChanged,[this](bool connected){
        if (connected) {
            m_subscription = m_pClient->createSubscription(100);
            qDebug()<<3;
            m_isConnected=true;
            //            subscribeObjects();
            //            m_monitorTimer->start(1800000);
            //            qDebug()<<4;
            m_monitorTimer->start(0);
        }
        else
            qDebug()<<"connected changed"<<connected;
    });


    QTimer *connectTimer=new QTimer;
    QTcpSocket *testSocket=new QTcpSocket;
    QObject::connect(connectTimer,&QTimer::timeout,[=](){
        //        qDebug()<<"OPCUA WATCHDOG search server";
        connectTimer->setInterval(OPCUA_INTERVAL);
        if (testSocket->bind(QHostAddress(OPCUA_IP),OPCUA_PORT)) {
            testSocket->close();
            if(m_isConnected){
                m_pClient->disconnectFromEndpoint();
                m_isConnected=false;
                m_monitorTimer->stop();
                m_triggerNode->deleteLater();
                m_triggerNode=0;
                m_subscription->deleteLater();
                for(auto &n:this->findChildren<QOpcUaNode*>())
                    n->deleteLater();
                for(auto &m:this->findChildren<QOpcUaMonitoredValue*>())
                    m->deleteLater();
                m_subscription=0;
                //                    m_pRoot->deleteLater();
                //                    m_pRoot=0;
                qDebug()<<"OPCUA Server disconnected";
            }
            //                qDebug()<<"OPCUA WATCHDOG testSocket->bind";
            //                if (m_pClient->isConnected()){

            //                    m_pClient->disconnectFromEndpoint();
            //                    emit m_pClient->connectedChanged(false);
            //                }
        }
        else {
            if (!m_isConnected){
                //                    if (m_pClient->isConnected()) {
                //                        qDebug()<<"OPCUA WATCHDOG m_pClient->isConnected()";
                //                        m_pClient->disconnectFromEndpoint();
                //                        while (!m_monitoredValues.isEmpty())
                //                            m_monitoredValues.takeLast()->deleteLater();
                //                        m_subscription->deleteLater();
                //                        m_subscription=0;
                //                    }
                qDebug()<<"OPCUA WATCHDOG server found";
                QTimer::singleShot(0,connectTimer,&QTimer::stop);
                QTimer::singleShot(100,this,//&OpcUaClient::serverFound);
                                   [=](){
                    qDebug()<<0;
                    if (!testSocket->bind(QHostAddress(OPCUA_IP),OPCUA_PORT))
                        emit serverFound();
                    connectTimer->start();
                    qDebug()<<8;
                });

                //                    emit serverFound();
            }
            //                else
            //                    qDebug()<<"OPCUA WATCHDOG m_isConnected is true";
        }
    });
    connectTimer->start(0);
}

OpcUaClient::~OpcUaClient()
{
    delete m_pClient;
    delete m_pProvider;
    //    delete m_inputCodeMonitorVal;
    delete m_subscription;
    //    delete m_monitorTimer;
    //    delete m_pRoot;
    //delete m_eventSubscription;
    m_objectsMap.clear();
}

void OpcUaClient::subscribeObjects()
{
    if(!m_triggerNode){
        m_triggerNode = m_pClient->node("ns=1;s=TriggerNode");
        if(m_triggerNode){
            QOpcUaMonitoredEvent *monitoredEvent=m_subscription->addEvent(m_triggerNode);
            if (monitoredEvent) {
                monitoredEvent->setObjectName("event_"+m_triggerNode->name().toLatin1());
                monitoredEvent->setProperty("nodename",m_triggerNode->name());
                monitoredEvent->setParent(m_triggerNode);
                qDebug()<<"Subscribe success to event"<<m_triggerNode->name()
                       <<"monitoredEvent"<<monitoredEvent;
                QObject::connect(monitoredEvent , &QOpcUaMonitoredEvent::newEvent,
                                 this,&OpcUaClient::subscribeObjects);//processEvent);
                qDebug()<<5;
            }
        }
    }
    QMapIterator<int,QString> objects(m_objectsMap);
    while (objects.hasNext()) {
        objects.next();
        qDebug()<<11;
        bool ok=true;
        if(m_isConnected)
            ok = ok && monitorNode(objects.key(),objects.value());
        if(ok)
            m_monitorTimer->stop();
        else
            m_monitorTimer->start(60000);
        qDebug()<<12;
    }
}

bool OpcUaClient::monitorNode(int deviceId, const QString &objectName)
{
    qDebug()<<12;
    //    if(m_pRoot && m_subscription){
    //        qDebug()<<"m_pRoot"<<m_pRoot<<"m_subscription"<<m_subscription;
    //        return;
    //    }
    //    QString nodeId = QString("ns=2;s=").append(objectName);
    //    if(!m_pRoot->childIds().contains(nodeId)){
    //        qDebug()<<"!m_pRoot->childIds().contains(nodeId)";
    //        return;
    //    }
    if (m_isConnected){
        QString nodeName(QString("ns=2;s=").append(objectName));
        QOpcUaNode* node = this->findChild<QOpcUaNode*>(nodeName);
        if (!node)
            node = m_pClient->node(nodeName);
        else
            qDebug()<<QString("Node %1 already exist").arg(objectName)<<node;
        if(node){
            /*
                QOpcUaMonitoredEvent *monitoredEvent=m_subscription->addEvent(node);
                if (monitoredEvent) {
                    monitoredEvent->setObjectName("event_"+node->name().toLatin1());
                    monitoredEvent->setProperty("nodename",node->name());
                    node->setProperty("nodename",node->name());
                    monitoredEvent->setParent(node);
                    qDebug()<<"Subscribe success to event"<<node->name()
                            <<"monitoredEvent"<<monitoredEvent;
                    //D:\Docs\Qt projects\Andon_project\souces\lib\
                    //qtopcua\src\plugins\opcua\freeopcua\qfreeopcuavaluesubscription.cpp
                    //        val.push_back(QVariant(QString::fromStdString(event.Message.Text)));
                    //        val.push_back(QVariant(QString::fromStdString(event.SourceName)));
                    //        val.push_back(QVariant((double) event.Severity));
                    QObject::connect(monitoredEvent , &QOpcUaMonitoredEvent::newEvent,
                                     this,&OpcUaClient::processEvent);
//                    QObject::connect(monitoredEvent, &QOpcUaMonitoredEvent::newEvent,
//                                     [monitoredEvent,this](QVector<QVariant> value){
//                                qDebug() << monitoredEvent->objectName()
//                                         << monitoredEvent->property("nodename").toString() << value;
//                    });
                }
                else
                    qDebug()<<"Failed subscribe to event"<<node->name();
//            for(QString propertyName:node->childIds())
//                if(propertyName.split(".").last()==constBAinputCode)
//        propertyNode->setProperty(constDevId,deviceId);
//        propertyNode->setProperty(constFullNodeName,propertyName);

*/
            node->setParent(this);
            node->setObjectName(nodeName);

            QStringList variables = node->childIds();
            qDebug()<<node<<node->name()<<variables.count()<<"properties";
            //        if(variables.contains(QString("ns=3;s=%1.%2").arg(objectName).arg(constBAinputCode)))
            for (QString p:variables){
                QString varName = p.split(".").last();
                if(varName==constBAinputCode || varName==constBAloaded){
                    QOpcUaNode* variable = this->findChild<QOpcUaNode*>(p);
                    if (!variable)
                        variable = m_pClient->node(p);
                    else
                        qDebug()<<QString("Variable %1 already exist").arg(p)<<variable;
                    if(variable){
                        variable->setParent(this);
                        variable->setObjectName(p);
                        subscribeProperty(deviceId, varName, variable);
                    } else qDebug()<<"Can not finde variable"<<p;
                }
            }
            QStringList vars;
            for (QString p:variables)
                vars<<p.split(".").last();
            if(!vars.contains(constBAinputCode)) {
                qDebug()<<"Can not finde variable inputCode in"<<objectName;
                for (auto p:variables)
                    qDebug()<<objectName<<p;
            }
            return true;
        }
        else {
            m_monitorTimer->start(360000);
            qDebug()<<"Can not finde object"<<objectName;
        }
    }
    return false;
}

bool OpcUaClient::subscribeProperty(int deviceId, const QString &variableName, QOpcUaNode *variable)
{
    qDebug()<<15;
    //    QString nodeName = variable->variable(constFullNodeName).toString();
    if(!m_subscription){
        qDebug()<<"m_subscription created";
        m_subscription = m_pClient->createSubscription(100);
    }
    if (variable) {
        QString monValName = QString("MonVal_").append(variable->objectName());
        QOpcUaMonitoredValue *monitoredValue =
                this->findChild<QOpcUaMonitoredValue *>(monValName);
        if (monitoredValue){
            qDebug() << variable << "Variable already subscribed"<<variable->name()<<variable<<monitoredValue;
            //TODO chack monitoredValue connection
            return true;
        }//
        //        qDebug()<<16<<variable->parent();
        //QOpcUaMonitoredValue *
        monitoredValue = m_subscription->addValue(variable);
        //            qDebug()<<17;
        if (monitoredValue) {
            monitoredValue->setObjectName(monValName);
            monitoredValue->setParent(this);
            qDebug()<<"Subscribe success to value"<<variable->name()<<variable<<monitoredValue;
            /*   lambda emit propertyChanged
//                QObject::connect(monitoredValue, &QOpcUaMonitoredValue::valueChanged,
//                                 [this,monitoredValue](QVariant value){
//                    qDebug()<<"valueChanged"<<monitoredValue<<value;
//                    emit propertyChanged(monitoredValue->variable(constDevId).toInt(),
//                                         monitoredValue->variable(constBAVariableName).toString(),value);
//                });
*/
            monitoredValue->setProperty(constDevId,deviceId);
            monitoredValue->setProperty(constBAVariableName,variableName);
            monitoredValue->setProperty(constBAVariableId,variable->objectName());

            QObject::connect(monitoredValue, &QOpcUaMonitoredValue::valueChanged,
                             this,&OpcUaClient::processValue);
            //                if(m_subscription->findChild<QOpcUaMonitoredValue *>(monValName))
            //                    qDebug()<< monValName <<"found in m_subscription";
            //                else
            //                    qDebug()<<QString("MonVal").append(variableName)<<"not found in m_subscription";
            return true;
        }
    }
    qDebug()<<"Can not subscribe to"<<variableName;
    return false;
}

void OpcUaClient::processValue(QVariant value)
{
    QOpcUaMonitoredValue *sender = qobject_cast<QOpcUaMonitoredValue *>(QObject::sender());
    if(sender) {
        QString pName(sender->property(constBAVariableName).toString());
        int deviceId=sender->property(constDevId).toInt();
        qDebug()<<sender->property(constBAVariableId).toString()<<value;
        emit propertyChanged(deviceId,pName,value);
        if(pName.isEmpty()){
            qDebug()<<"Empty property!!!"<<sender;
            pName=constBAinputCode;
            sender->deleteLater();
            subscribeObjects();
        }
    } else qDebug()<<"bad casting of sender";
}

void OpcUaClient::processEvent(QVector<QVariant> value)
{
    QOpcUaMonitoredEvent *sender = qobject_cast<QOpcUaMonitoredEvent *>(QObject::sender());
    if(sender) {
        qDebug()<<sender->property("nodename").toString()<<sender<<value;
    } else qDebug()<<"bad casting of sender";
    subscribeObjects();
}

void OpcUaClient::appendObject(int deviceId, const QString &objectName)
{
    qDebug()<<10;
    m_objectsMap.insert(deviceId,objectName);
    if (m_isConnected)
        monitorNode(deviceId,objectName);
}
