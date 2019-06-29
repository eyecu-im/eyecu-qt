#include "signalsession.h"
#include "signalprotocol.h"
#include <session_builder.h>
#include <session_cipher.h>
#include <protocol.h>

namespace SignalSession {

signal_context *				FGlobalContext(nullptr);
signal_protocol_store_context *	FStoreContext(nullptr);

void init(signal_context *AGlobalContext,
		  signal_protocol_store_context *AStoreContext)
{
	FGlobalContext = AGlobalContext;
	FStoreContext = AStoreContext;
}

SessionBuilder::SessionBuilder(const QString &ABareJid, int ADeviceId):
	ABuilder(nullptr)
{
	signal_protocol_address AAddress = {ABareJid.toUtf8(),
										size_t(ABareJid.size()),
										ADeviceId};
	// Instantiate a session_builder for a recipient address.
	int rc = session_builder_create(&ABuilder, FStoreContext, &AAddress, FGlobalContext);
	if (rc != SG_SUCCESS)
		qCritical("session_builder_create() failed! rc=%d", rc);
}

SessionBuilder::~SessionBuilder()
{
	if (ABuilder)
		session_builder_free(ABuilder);
}

bool SessionBuilder::isOk() const
{
	return ABuilder != nullptr;
}

bool SessionBuilder::processPreKeyBundle(session_pre_key_bundle *APreKey)
{
	// Build a session with a pre key retrieved from the server.
	int rc = session_builder_process_pre_key_bundle(ABuilder, APreKey);
	if (rc == SG_SUCCESS)
		return true;
	else
	{
		qCritical("session_builder_process_pre_key_bundle() failed! rc=%d", rc);
		return false;
	}
}

session_cipher *sessionCipherCreate(const QString &ABareJid, int ADeviceId)
{
	signal_protocol_address AAddress = {ABareJid.toUtf8(),
										size_t(ABareJid.size()),
										ADeviceId};

	// Create the session cipher
	session_cipher	*cipher(nullptr);
	int rc = session_cipher_create(&cipher, FStoreContext, &AAddress, FGlobalContext);
	if (rc != SG_SUCCESS)
		qCritical("session_builder_process_pre_key_bundle() failed! rc=%d", rc);
	return cipher;
}

QByteArray encrypt(session_cipher *ACipher, const QByteArray &AUnencrypted)
{
	QByteArray result;
	ciphertext_message *message;
	int rc = session_cipher_encrypt(ACipher,
									reinterpret_cast<const quint8*>(
										AUnencrypted.data()),
									size_t(AUnencrypted.size()),
									&message);
	if (rc == SG_SUCCESS)
	{
		// Get the serialized content
		signal_buffer *serialized = ciphertext_message_get_serialized(message);
		result = SignalProtocol::signalBufferToByteArray(serialized);
		signal_buffer_free(serialized);
	}

	SIGNAL_UNREF(message);

	return result;
}

QByteArray decrypt(session_cipher *ACipher, const QByteArray &AEncrypted)
{
	signal_message *ciphertext;
	QByteArray result;

	int rc = signal_message_deserialize(&ciphertext,
										reinterpret_cast<const quint8*>(
											AEncrypted.data()),
										size_t(AEncrypted.size()),
										FGlobalContext);
	if (rc == SG_SUCCESS)
	{
		signal_buffer *buffer(nullptr);

		rc = session_cipher_decrypt_signal_message(ACipher, ciphertext,
												   nullptr, &buffer);
		if (rc == SG_SUCCESS)
			result = SignalProtocol::signalBufferToByteArray(buffer);
		else
			qCritical("session_cipher_decrypt_signal_message() failed! rc=%d", rc);

		signal_buffer_bzero_free(buffer);
	}
	else
		qCritical("signal_message_deserialize() failed! rc=%d", rc);

	return result;
}

QByteArray decryptPre(session_cipher *ACipher, const QByteArray &AEncrypted)
{
	pre_key_signal_message *ciphertext(nullptr);
	QByteArray result;
	int rc = pre_key_signal_message_deserialize(&ciphertext,
												reinterpret_cast<const quint8*>(
													AEncrypted.data()),
												size_t(AEncrypted.size()),
												FGlobalContext);
	if (rc == SG_SUCCESS)
	{
		signal_buffer *plaintext(nullptr);

		rc = session_cipher_decrypt_pre_key_signal_message(ACipher, ciphertext,
														   nullptr, &plaintext);
		if (rc == SG_SUCCESS)
		{
			result = SignalProtocol::signalBufferToByteArray(plaintext);
		}
		else
			qCritical("session_cipher_decrypt_signal_message() failed! rc=%d", rc);

		signal_buffer_bzero_free(plaintext);
	}
	else
		qCritical("pre_key_signal_message_deserialize() failed! rc=%d", rc);

	return result;
}

}
