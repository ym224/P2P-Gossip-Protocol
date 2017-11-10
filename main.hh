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


class NetSocket : public QUdpSocket
{
	Q_OBJECT

public:
	NetSocket();

	// Bind this socket to a P2Papp-specific default port.
	bool bind();
	int getPort();
	int getMyPortMin();
	int getMyPortMax();
	
	void sendData(QByteArray bytes, quint32 receiverPort);
	QVariantMap readData(QHostAddress senderAddr, quint16 senderPort);

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
	QVariantMap lastRumorMessage;
	QVariantMap statusMessage;
	QVector<int> neighbors; //nodes on adjacent ports
	quint32 seqNum;
	QString originId;
	bool timer_started;

	//need to store a map of read originId to a map of (seqnum, message)
	QMap<QString, QMap<quint32, QVariantMap> > messages;
	void addNeighbors();
	QVariantMap createStatusMessage(QString origin, quint32 seqNum);
	QVariantMap createRumorMessage(QString chatText, QString origin, quint32 seqNum);
	bool updateStatusMessage(QString origin, quint32 seqNum);
	void processRumorMessage(QVariantMap rumorMessage, quint16 senderPort);
	void processStatusMessage(QVariantMap peerStatusMessage, quint16 senderPort);
	void rumorMonger(QVariantMap message);
	void sendMessage(QString text); 
	QString generateOriginId(int port);
	QByteArray serializeMessage(QVariantMap message);
};

#endif // P2PAPP_MAIN_HH
