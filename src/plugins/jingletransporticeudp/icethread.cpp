#include <QUuid>
#include "icethread.h"

IceThread::IceThread(const QPIceTransport::Config &AIceCfg,
					 QPIceSession::Role AIceRole, IJingleContent *AContent,
					 QObject *AParent):
	QThread (AParent),
	FIceConfig(AIceCfg),
	FIceRole(AIceRole),
	FLastState(QPIceTransport::StateNull),
	FLastStatus(QP_NO_ERROR),
	FLocalUfrag(QUuid::createUuid().toString()),
	FLocalPwd(QUuid::createUuid().toString()),
	FSid(AContent->sid()),
	FContentName(AContent->name()),
	FComponentCount(AContent->componentCount())
{
	setObjectName(AContent->name());
	moveToThread(this);
}

IceThread::~IceThread()
{

}

int IceThread::status() const
{
	return FLastStatus;
}

QPIceTransport::State IceThread::state() const
{
	return FIceTransport?FIceTransport->state():FLastState;
}

QPIceSession::Role IceThread::role() const
{
	return FIceTransport?FIceTransport->role():FIceRole;
}

int IceThread::componentCount() const
{
	return FIceTransport?FIceTransport->componentCount():0;
}

QPIceComponent *IceThread::component(int ACompId) const
{
	return FIceTransport?FIceTransport->runningComponents().value(ACompId):nullptr;
}

int IceThread::enumCandidates(int ACompId, QHash<QString, QPIceCandidate> &ACand) const
{
	return FIceTransport?FIceTransport->enumCandidates(ACompId, ACand):QP_EINVAL;
}

bool IceThread::startIce(const QString &ARemoteUFrag, const QByteArray &ARemotePassword,
						 const QHash<QString, QPIceCandidate> &ARemoteCandidates)
{
	if (FIceTransport) {
		FRemCands = ARemoteCandidates;
		emit iceStart(ARemoteUFrag, ARemotePassword);
		return true;
	}
	return false;

}

void IceThread::destroy()
{
	emit iceDestroy();
}

IJingleContent *IceThread::content() const
{
	return FContent;
}

int IceThread::initIce()
{
	int status;

	FIceTransport = new QPIceTransport(FIceConfig, FContent->componentCount(), &status);

	if (status != QP_NO_ERROR)
		return status;

	// Connect SIGNALS/SLOTS
	connect(FIceTransport, SIGNAL(complete(QPIceTransport::Operation,int)),
						   SLOT(onIceComplete(QPIceTransport::Operation,int)));

	connect(FIceTransport, SIGNAL(destroyed(QObject *)), SLOT(onIceDestroyed(QObject *)));

	connect(this, SIGNAL(iceStart(QString,QByteArray)), SLOT(onIceStart(QString,QByteArray)));

	connect(this, SIGNAL(iceDestroy()), SLOT(onIceDestroy()));

	return QP_NO_ERROR;
}

void IceThread::run()
{
	FLastStatus = initIce();

	if (FLastStatus != QP_NO_ERROR)
		return;

	exec();
}

void IceThread::onIceComplete(QPIceTransport::Operation AOperation, int ALastStatus)
{
	qDebug() << "IceThread::onIceComplete(" << AOperation << "," << ALastStatus << ")";
	if (ALastStatus == QP_NO_ERROR) {
		if (AOperation == QPIceTransport::OperationInit)
			ALastStatus = FIceTransport->initIce(FIceRole, FLocalUfrag, FLocalPwd);

		if (ALastStatus == QP_NO_ERROR) {
			if (FIceTransport->state() == QPIceTransport::StateRunning) {
//TODO: Emit correct signal
			}
			emit iceSuccess(AOperation);
			return;
		}
	}

	qDebug() << "status string:" << QpErrno::errorString(ALastStatus);

	FLastStatus = ALastStatus;
	FLastState = FIceTransport->state();
	FIceTransport->destroy();
}

void IceThread::onIceDestroyed(QObject *)
{
	qDebug() << "IceThread::onIceDestroyed()";
	exit();
}

void IceThread::onIceStart(const QString &ARemoteUFrag, const QByteArray &ARemotePassword)
{
	if (FIceTransport)
		FIceTransport->startIce(ARemoteUFrag, ARemotePassword, FRemCands);
}

void IceThread::onIceDestroy()
{
	qDebug() << "IceThread::onIceDestroy()";
	if (FIceTransport)
		FIceTransport->destroy();
	else
		exit();
}
