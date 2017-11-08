#ifndef P2PAPP_MAIN_HH
#define P2PAPP_MAIN_HH

#include <QDialog>
#include <QTextEdit>
#include <QLineEdit>
#include <QUdpSocket>
#include <QTimer>
#include <QDebug>

const long TIMEOUT = 1000;
const long ANTIENTROPY_TIMEOUT = 10000;
const qint32 STANDBY = 1;
const qint32 WAITING_ACK = 2;

class Gossip
{

public:
	Gossip();
	// status message
	Gossip(QString origin, quint32 seqNum); 
	// rumor message
	Gossip(QString chatText, QString origin, quint32 seqNum);
	void updateStatus(QString origin, quint32 seqNum);
	
	QVariantMap *rumorMessage;
	QVariantMap *statusMessage;
	QVariantMap *statusValue;
};

class NetSocket : public QUdpSocket
{
	Q_OBJECT

public:
	NetSocket();

	// Bind this socket to a P2Papp-specific default port.
	int bind();
	void sendData(Gossip *message, quint32 receiverPort);
	void readData(QHostAddress senderAddr, quint16 senderPort);

private:
	int myPortMin, myPortMax;
};


class ChatDialog : public QDialog
{
	Q_OBJECT

public:
	ChatDialog();

public slots:
	void gotReturnPressed();
	void readDatagrams();
	void timeoutHandler();
	void antiEntropyHandler();

private:
	QTextEdit *textview;
	QLineEdit *textline;
	QTimer *timer;
	QTimer *antiEntropyTimer;	
	NetSocket *socket;
	quint32 seqNum;

	
	void processDatagram(QByteArray bytes);
	void processRumorMessage(QVariantMap message);
	void processStatus(QVariantMap message);
	void rumorMonger(QVariantMap message);
	QByteArray serializeMessage(QString);
	QByteArray serializeStatus();
};

#endif // P2PAPP_MAIN_HH