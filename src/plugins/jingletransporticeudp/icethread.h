#ifndef ICETHREAD_H
#define ICETHREAD_H

#include <QThread>
#include <QPointer>
#include <QPIceTransport>

#include <interfaces/ijingle.h>

class IceThread : public QThread
{
	Q_OBJECT

public:
	IceThread(const QPIceTransport::Config &AIceCfg, QPIceSession::Role AIceRole,
			  IJingleContent * AContent, QObject *AParent=nullptr);

	virtual ~IceThread();

	int status() const;
	QPIceTransport::State state() const;
	QPIceSession::Role role() const;
	int componentCount() const;
	QPIceComponent *component(int ACompId) const;
	int enumCandidates(int ACompId, QHash<QString, QPIceCandidate> &ACand) const;
	bool startIce(const QString &ARemoteUFrag, const QByteArray &ARemotePassword,
				  const QHash<QString, QPIceCandidate> &ARemoteCandidates);
	void destroy();
	IJingleContent *content() const;

	const QString &localUfrag() const {return FLocalUfrag;}
	const QString &localPwd() const {return FLocalPwd;}

protected:
	int initIce();

	// QThread interface
	virtual void run() override;

protected slots:
	void onIceComplete(QPIceTransport::Operation AOperation, int ALastStatus);
	void onIceDestroyed(QObject *);
	void onIceStart(const QString &ARemoteUFrag, const QByteArray &ARemotePassword);
	void onIceDestroy();

signals:
	void iceSuccess(int AOperation);
	void iceStart(const QString &ARemoteUFrag, const QByteArray &ARemotePassword);
	void iceDestroy();

private:
	QPointer<QPIceTransport> FIceTransport;
	QPIceTransport::Config FIceConfig;
	QPIceSession::Role FIceRole;
	QPIceTransport::State FLastState;
	int FLastStatus;

	QString FLocalUfrag;
	QString FLocalPwd;

	QHash<QString, QPIceCandidate> FRemCands;
	IJingleContent	*FContent;
};

#endif // ICETHREAD_H
