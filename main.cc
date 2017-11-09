
#include <unistd.h>

#include <QVBoxLayout>
#include <QApplication>
#include <QDebug>

#include "main.hh"

ChatDialog::ChatDialog()
{
	setWindowTitle("P2Papp");

	// Read-only text box where we display messages from everyone.
	// This widget expands both horizontally and vertically.
	textview = new QTextEdit(this);
	textview->setReadOnly(true);

	// Small text-entry box the user can enter messages.
	// This widget normally expands only horizontally,
	// leaving extra vertical space for the textview widget.
	//
	// You might change this into a read/write QTextEdit,
	// so that the user can easily enter multi-line messages.
	textline = new QLineEdit(this);

	// Lay out the widgets to appear in the main window.
	// For Qt widget and layout concepts see:
	// http://doc.qt.nokia.com/4.7-snapshot/widgets-and-layouts.html
	QVBoxLayout *layout = new QVBoxLayout();
	layout->addWidget(textview);
	layout->addWidget(textline);
	setLayout(layout);

	// Create a UDP network socket
	socket = new NetSocket();
	if (!socket->bind()) {
		exit(1);
	}
	
	originId = generateOriginId(socket->getPort());
	addNeighbors();
	
	// initialize timeout timer
	timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(timeoutHandler()));

	// initialize anti-entropy timer
	antiEntropyTimer = new QTimer(this);
	connect(antiEntropyTimer, SIGNAL(timeout()), this, SLOT(antiEntropyHandler()));

	this->seqNum = 0;
	
	// Register a callback on the textline's returnPressed signal
	// so that we can send the message entered by the user.
	connect(textline, SIGNAL(returnPressed()), this, SLOT(gotReturnPressed()));

	// connect the readyRead signal in QUdpSocket
	connect(socket, SIGNAL(readyRead()), this, SLOT(readDatagrams()));
}


void ChatDialog::addNeighbors()
{
	int port = socket->getPort();
	if (port == socket->getMyPortMin()){
		neighbors.append(port + 1);
		qDebug() << "INFO: adding neighbor: " << neighbors.first();
	}
	if (port == socket->getMyPortMax()){
		neighbors.append(port - 1);
		qDebug() << "INFO: adding neighbor: " << neighbors.first();		
	}
	if (port < socket->getMyPortMax() && port > socket->getMyPortMin()){
		neighbors.append(port - 1);
		qDebug() << "INFO: adding neighbor: " << neighbors.first();				
		neighbors.append(port + 1);
		qDebug() << "INFO: adding neighbor: " << neighbors.last();				
	}
}

// called when a new payload of network data has arrived on socket
void ChatDialog::readDatagrams()
{
	QHostAddress senderAddr;
	quint16 senderPort;
	// receive and serialize data from socket
	socket->readData(senderAddr, senderPort);
	
	// check if rumorMessage of statusMessage
	// if statusMessage, compare with own status
	// if you have message not seen by peer, rumorMonger
	// if peer has message not seen by you, send status 
	if (message->contains("Want")){
		qDebug() << "INFO: Received a status message";
		// process statusMessage
		QVariantMap *peerStatusValue = message->statusValue;
		QVariantMap *statusValue = statusMessage->statusValue;
		processStatusMessage(peerStatusValue, statusValue);
	}
	// if rumorMessage, add to infoMap
	// randomly rumorMonger or stop
	else {
		qDebug() << "INFO: Received a rumor message";
		processRumorMessage();
	}
	
}

void ChatDialog::processStatusMessage(QVariantMap *peerStatusValue)
{
	// local ahead of remote, send new rumor
	// local behind remote, send status
	// local in sync with remote, rumorMonger

}

void ChatDialog::processRumorMessage(QVariantMap *rumorMessage)
{
	// get origin and seqnum
	// terminate if origin is self
	
	// create new rumor message
	Gossip *gossip = new Gossip(text, origin, recSeqNum);
	// insert rumor Message to local infoMap
	
	// update status message if behind
	
	// rumorMonger with received message
	
	// add text to textView
	
	// send statusMessage as ack
}
 
QString ChatDialog::generateOriginId(int port)
{	
	int rand_val = qrand();

	QString qString(QHostInfo::localHostName() + port + rand_val);
	qDebug() << "INFO: generated origin id: " << qString;
	return qString;
}

void ChatDialog::gotReturnPressed()
{
	// Initially, just echo the string locally.
	// Insert some networking code here...
	qDebug() << "FIX: send message to other peers: " << textline->text();
	textview->append(textline->text());
	sendMessage(textline->text());
	
	// Clear the textline to get ready for the next input message.
	textline->clear();
}

void ChatDialog::sendMessage(QString text)
{
	QString origin = generateOriginId(socket->getPort());
	Gossip *message = new Gossip(text, origin, this->seqNum);
	
	// add own originId, seqNum and message to map
	QMap infoMap = new QMap<QString, QMap<quint32, RumorMessage*>*>();
	QMap seqMessageMap = new QMap<quint32, RumorMessage*>();
	seqMessageMap->insert(this->seqNum, message);
	infoMap->insert(this->name, seqMessageMap); 
	
	// update own status message
	statusMessage.updateStatus(this->name, this->seqNum);
	
	// rumor monger to random neighbor
	rumorMonger(message);
	
}

void ChatDialog::rumorMonger(Gossip *message)
{
	// pick a random neighbor and send rumorMessage
	quint32 receiverPort = (quint32)neighbors.at(qrand() % neighbors.size());
	qDebug() << "Gossiping to neighbor " << receiverPort;
	socket->sendData(message, socket->getPort());
	lastRumorMessage = message;
	this->timer->start(TIMEOUT);
	// wait for statusMessage until timeout
}

void ChatDialog::timeoutHandler()
{
	qDebug() << "INFO: in timeoutHandler";
	// resend last message to last communicated node
	socket->sendData(lastRumorMessage, socket->getPort());
	// reset timer
	timer->start(TIMEOUT);
}


void ChatDialog::antiEntropyHandler()
{
	qDebug() << "INFO: in antiEntropyHandler()";
	Gossip *status;
	int rand_idx = qrand() % neighbors.size();
	socket->sendData(status, (quint32)neighbors.at(rand_idx));

	// reset timer
	antiEntropyTimer->start(ANTIENTROPY_TIMEOUT);
}


NetSocket::NetSocket()
{
	// Pick a range of four UDP ports to try to allocate by default,
	// computed based on my Unix user ID.
	// This makes it trivial for up to four P2Papp instances per user
	// to find each other on the same host,
	// barring UDP port conflicts with other applications
	// (which are quite possible).
	// We use the range from 32768 to 49151 for this purpose.
	myPortMin = 32768 + (getuid() % 4096)*4;
	myPortMax = myPortMin + 3;
}

bool NetSocket::bind()
{
	// Try to bind to each of the range myPortMin..myPortMax in turn.
	for (int p = myPortMin; p <= myPortMax; p++) {
		if (QUdpSocket::bind(p)) {
			qDebug() << "INFO: bound to UDP port " << p;
			port = p;
			return true;
		}
	}

	qDebug() << "Oops, no ports in my default range " << myPortMin
		<< "-" << myPortMax << " available";
	return false;
}

void NetSocket::sendData(Gossip *message, quint32 receiverPort)
{
	QByteArray bytes;
	// serialize message using QDataStream
	QDataStream stream(&bytes, QIODevice::WriteOnly);
	stream << (message->rumorMessage);
	qDebug() << "Sending " << message->rumorMessage << " to " << receiverPort << bytes.size();
	writeDatagram(bytes, QHostAddress(QHostAddress::LocalHost), receiverPort);
}

void NetSocket::readData(QHostAddress senderAddr, quint16 senderPort)
{
	while (this->hasPendingDatagrams()) {
		QByteArray bytes;
		bytes.resize(this->pendingDatagramSize());
		QVariantMap *rumorMessage = new QVariantMap;
		this->readDatagram(bytes.data(), bytes.size(), &senderAddr, &senderPort);
		// deserialize
		QDataStream stream(&bytes, QIODevice::ReadOnly);
		qDebug() << "Receiving " << *rumorMessage << " from " << senderPort;
		stream >> *rumorMessage;
		//TODO: return rumorMessage
	}
}

Gossip::Gossip(QString chatText, QString origin, quint32 seqNum)
{
	rumorMessage = new QVariantMap();
	rumorMessage->insert("ChatText", chatText);
	rumorMessage->insert("Origin", origin);
	this->rumorMessage->insert("SeqNo", seqNum);
}

Gossip::Gossip(QString origin, quint32 seqNum)
{
	statusMessage = new QVariantMap();
	statusValue = new QVariantMap();
	statusValue->insert(origin, seqNum);
	statusMessage->insert("Want", *statusValue);
}

void Gossip::updateStatus(QString origin, quint32 seqNum)
{
	if (statusValue->contains(origin) || seqNum == 1) {
		statusValue->insert(origin, seqNum + 1);
		statusMessage->insert("Want", *statusValue);
	}
} 

int main(int argc, char **argv)
{
	// Initialize Qt toolkit
	QApplication app(argc,argv);

	// Create an initial chat dialog window
	ChatDialog dialog;
	dialog.show();

	// Enter the Qt main loop; everything else is event driven
	return app.exec();
}

