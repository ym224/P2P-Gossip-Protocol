#ifndef P2PAPP_MAIN_HH
#define P2PAPP_MAIN_HH

#include <QDialog>
#include <QTextEdit>
#include <QLineEdit>
#include <QUdpSocket>
#include <QTimer>
#include <QDebug>
#include <QHostInfo>

const long TIMEOUT = 1000;
const long ANTIENTROPY_TIMEOUT = 10000;
const qint32 STANDBY = 1;
const qint32 WAITING_ACK = 2;

class Gossip
{

public:
	Gossip();
	// constructor for status message
	Gossip(QString origin, quint32 seqNum); 
	// construtor for rumor message
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
	bool bind();
	int getPort() { return port; }
	int getMyPortMin() { return myPortMin; }
	int getMyPortMax() { return myPortMax; }
	
	void sendData(Gossip *message, quint32 receiverPort);
	void readData(QHostAddress senderAddr, quint16 senderPort);

private:
	int myPortMin, myPortMax, port;
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
	Gossip *lastRumorMessage;
	Gossip *statusMessage;
	QVector<int> neighbors; //nodes on adjacent ports
	quint32 seqNum;
	QString originId;

	//need to store a map of peer originId to a map of (seqnum, gossip message) 
	void addNeighbors();
	void processDatagram(QByteArray bytes);
	void processRumorMessage(QVariantMap message);
	void processStatusMessage(QVariantMap message);
	void rumorMonger(Gossip *message);
	void sendMessage(QString text);
	QString generateOriginId(int port);
	QByteArray serializeMessage(QString);
	QByteArray serializeStatus();
};

#endif // P2PAPP_MAIN_HH
