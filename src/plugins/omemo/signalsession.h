#ifndef SIGNALSESSION_H
#define SIGNALSESSION_H

#include <signal_protocol_types.h>
#include <QString>

struct signal_protocol_store_context;
struct session_builder;
struct signal_context;
struct session_pre_key_bundle;
struct session_cipher;
class SignalProtocol;

namespace SignalSession {

extern signal_context *					FGlobalContext;
extern signal_protocol_store_context *	FStoreContext;

void init(signal_context *AGlobalContext,
	signal_protocol_store_context *AStoreContext);

session_cipher *sessionCipherCreate(const QString &ABareJid, int ADeviceId);

QByteArray encrypt(session_cipher *ACipher, const QByteArray &AUnencrypted);
QByteArray decrypt(session_cipher *ACipher, const QByteArray &AEncrypted);
QByteArray decryptPre(session_cipher *ACipher, const QByteArray &AEncrypted);

class SessionBuilder
{
public:
	SessionBuilder(const QString &ABareJid, int ADeviceId);
	~SessionBuilder();

	bool processPreKeyBundle(session_pre_key_bundle *APreKey);

	bool isOk() const;
private:
	session_builder	*ABuilder;
};

}

#endif // SIGNALSESSION_H
