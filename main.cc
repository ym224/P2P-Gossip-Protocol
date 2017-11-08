
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

	// bind socket
	socket = new NetSocket();
	if (!socket->bind()) {
		exit(1);
	}
	
	// initialize timeout timer
	timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(handleTimeout()));

	// initialize anti-entropy timer
	antiEntropyTimer = new QTimer(this);
	connect(antiEntropyTimer, SIGNAL(timeout()), this, SLOT(handleAntiEntropyTimeout()));
	antiEntropyTimer->start(ANTIENTROPY_TIMEOUT);
	
	// Register a callback on the textline's returnPressed signal
	// so that we can send the message entered by the user.
	connect(textline, SIGNAL(returnPressed()),
		this, SLOT(gotReturnPressed(
		
		)));

	// connect the readyRead signal in QUdpSocket
	connect(socket, SIGNAL(readyRead()), this, SLOT(readDatagrams()));

	this->seqNum = 0;
}

// called when a new payload of network data has arrived on socket
void ChatDialog::readDatagrams()
{
	while (this.socket->hasPendingDatagrams()) {
		QByteArray bytes;
		bytes.resize(socket->pendingDatagramSize());
		QHostAddress sender;
		quint16 senderPort;

		socket->readDatagram(bytes.data(), bytes.size(), &sender, &senderPort);
		processDatagram(bytes);
	}
}

void ChatDialog::processDatagram(QByteArray bytes){
	QVariantMap messageMap;

}
 
QString ChatDialog::generateOriginId(int port)
{
	QString qString('Origin' + port);
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
	Gossip *message = new Gossip(text, this->socket, this.seqnum)
	
}

void ChatDialog::rumorMonger(QVariantMap message)
{

}

void ChatDialog::timeoutHandler()
{
	qDebug() << "INFO: in timeoutHandler";
	// resent last message to last communicated node
	// reset timer
	timer->start(1000);
}


void ChatDialog::antiEntropyHandler()
{
	qDebug() << "INFO: in antiEntropyHandler()";
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

int NetSocket::bind()
{
	// Try to bind to each of the range myPortMin..myPortMax in turn.
	for (int p = myPortMin; p <= myPortMax; p++) {
		if (QUdpSocket::bind(p)) {
			qDebug() << "bound to UDP port " << p;
			return p;
		}
	}

	qDebug() << "Oops, no ports in my default range " << myPortMin
		<< "-" << myPortMax << " available";
	return -1;
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

	// Create a UDP network socket
	NetSocket sock;
	if (!sock.bind())
		exit(1);

	// Enter the Qt main loop; everything else is event driven
	return app.exec();
}

