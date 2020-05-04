#ifndef SIGNALPROTOCOL_H
#define SIGNALPROTOCOL_H

#include <QMap>
#include <QSharedData>
#include <QSharedDataPointer>
#include <signal_protocol.h>

#include "omemostore.h"

class QMutex;

struct session_builder;

#define SIGNED_PRE_KEY_ID	1
#define PRE_KEYS_START		1
#define PRE_KEYS_AMOUNT		100

#define ADDR_NAME(X) QString::fromLatin1(QByteArray(X->name, int(X->name_len)))
#define DATA_SIZE(A) reinterpret_cast<const quint8*>(A.data()), size_t(A.size())

#define BADS(D,S) reinterpret_cast<const char*>(D), int(S)
#define BYTE_ARRAY(D,S) QByteArray(BADS(D,S))
#define SBUF2BARR(SB) BYTE_ARRAY(signal_buffer_data(SB), \
								 signal_buffer_len(SB))

class SignalProtocol
{
public:
	enum SessionState {
		NoSession,
		SessionInitiated,
		SessionAcknowledged
	};

	class IIdentityKeyListener {
	public:
		virtual bool onNewKeyReceived(const QString &AName, const QByteArray &AKeyData) = 0;
	};

	class SignalMessage {
		friend class SignalProtocol;

	public:
		SignalMessage(const SignalMessage &AOther);
		~SignalMessage();
		bool isNull() const;

		SignalMessage operator = (const SignalMessage &AOther);
		bool operator == (const SignalMessage &AOther) const;
		bool operator != (const SignalMessage &AOther) const;

	protected:
		SignalMessage(signal_context *AGlobalContext, const QByteArray &AEncrypted);
		operator signal_message *() const;

	private:
		signal_message *FMessage;
	};

	class PreKeySignalMessage {
		friend class SignalProtocol;

	public:
		PreKeySignalMessage(const PreKeySignalMessage &AOther);
		~PreKeySignalMessage();
		bool isNull() const;

		PreKeySignalMessage operator = (const PreKeySignalMessage &AOther);
		bool operator == (const PreKeySignalMessage &AOther) const;
		bool operator != (const PreKeySignalMessage &AOther) const;

		bool hasPreKeyId() const;
		quint32 preKeyId() const;

	protected:
		PreKeySignalMessage(signal_context *AGlobalContext, const QByteArray &AEncrypted, quint32 ARegistrationId);
		operator pre_key_signal_message *() const;

	private:
		pre_key_signal_message *FMessage;
	};

	class Cipher {
		friend class SignalProtocol;

	public:
		Cipher(const Cipher &AOther);
		~Cipher();
		bool isNull() const;

		QByteArray encrypt(const QByteArray &AUnencrypted);
		QByteArray decrypt(const SignalMessage &AMessage);
		QByteArray decrypt(const PreKeySignalMessage &AMessage, bool &APreKeysUpdated);

		Cipher operator = (const Cipher &AOther);
		bool operator == (const Cipher &AOther) const;
		bool operator != (const Cipher &AOther) const;

	protected:
		Cipher(SignalProtocol *ASignalProtocol, const QString &ABareJid, int ADeviceId, quint32 AVersion);
	private:
		SignalProtocol *FSignalProtocol;
		session_cipher *FCipher;
		QByteArray FBareJid;
		signal_protocol_address FAddress;		
	};

	class SessionBuilderData: public QSharedData
	{
	public:
		SessionBuilderData(const QString &ABareJid, quint32 ADeviceId);
		SessionBuilderData(const SessionBuilderData &AOther);
		~SessionBuilderData();

		QByteArray	FBareJid;
		signal_protocol_address FAddress;
		session_builder	*FBuilder;
	};

	class SessionBuilder
	{
		friend class SignalProtocol;

	public:
		SessionBuilder(const SessionBuilder &AOther);
		~SessionBuilder();

		bool processPreKeyBundle(session_pre_key_bundle *APreKey);
		bool isNull() const;

	protected:
		SessionBuilder(const QString &ABareJid, quint32 ADeviceId,
					   signal_context *AGlobalContext,
					   signal_protocol_store_context *AStoreContext, quint32 AVersion);
	private:
		QSharedDataPointer<SessionBuilderData> d;
	};

	SignalProtocol(const QString &AFileName, const QString &AConnectionName, IIdentityKeyListener *AIdentityKeyListener, quint32 AVersion);
	~SignalProtocol();

	static void init();

	static QString calcFingerprint(const QByteArray &APublicKey);

	static QByteArray signalBufferToByteArray(signal_buffer *ABuffer);

	QString connectionName() const;

	bool onNewKeyReceived(const QString &AName, const QByteArray &AKeyData);

	signal_context *globalContext() const;
	signal_protocol_store_context *storeContext() const;

	int error() const;	

	int install(quint32 ASignedPreKeyId=SIGNED_PRE_KEY_ID, uint APreKeyStartId=PRE_KEYS_START, uint APreKeyAmount=PRE_KEYS_AMOUNT);

	quint32 getDeviceId();

	int sessionInitStatus(const QString &ABareJid, qint32 ADeviceId);
	Cipher sessionCipherCreate(const QString &ABareJid, int ADeviceId);

	QByteArray getIdentityKeyPublic() const;
	QByteArray getIdentityKeyPrivate() const;

	QByteArray getSignedPreKeyPublic(quint32 AKeyId=SIGNED_PRE_KEY_ID) const;
	QByteArray getSignedPreKeySignature(quint32 AKeyId=SIGNED_PRE_KEY_ID) const;

	QByteArray getPreKeyPublic(quint32 AKeyId) const;
	QByteArray getPreKeyPrivate(quint32 AKeyId) const;

	QMap<quint32, QByteArray> getPreKeys() const;

	QList<OmemoStore::IdentityKey> getIdentityKeys() const;

	session_pre_key_bundle *createPreKeyBundle(uint32_t ARegistrationId,
											   int ADeviceId, uint32_t APreKeyId,
											   const QByteArray &APreKeyPublic,
											   uint32_t ASignedPreKeyId,
											   const QByteArray &ASignedPreKeyPublic,
											   const QByteArray &ASignedPreKeySignature,
											   const QByteArray &AIdentityKey) const;

	SignalMessage getSignalMessage(const QByteArray &AEncrypted);
	PreKeySignalMessage getPreKeySignalMessage(const QByteArray &AEncrypted);
	SessionBuilder getSessionBuilder(const QString &ABareJid, quint32 ADeviceId);
#define HASH_OUTPIT_SIZE 32
	QByteArray hkdf_gen(int ALength, const QByteArray &AIkm, const QByteArray &AInfo=QByteArray(), const QByteArray &ASalt=QByteArray(HASH_OUTPIT_SIZE, 0));

protected:
	int generateIdentityKeyPair(ratchet_identity_key_pair **AIdentityKeyPair);
	int generateRegistrationId(quint32 *ARegistrationId, int AExtendedRange);
	int generatePreKeys(signal_protocol_key_helper_pre_key_list_node **APreKeyList, uint AStart, uint ACount);
	int generateSignedPreKey(session_signed_pre_key **ASignedPreKey, const ratchet_identity_key_pair *AIdentityKeyPair, uint32_t ASignedPreKeyId, uint64_t ATimestamp);

	/**
	 * Callback for a secure random number generator.
	 * This function shall fill the provided buffer with random bytes.
	 *
	 * @param AData pointer to the output buffer
	 * @param ALen size of the output buffer
	 * @return 0 on success, negative on failure
	 */
	static int randomFunc(uint8_t *AData, size_t ALen, void *AUserData);

	/**
	 * Callback for an HMAC-SHA256 implementation.
	 * This function shall initialize an HMAC context with the provided key.
	 *
	 * @param AHmacContext private HMAC context pointer
	 * @param AKey pointer to the key
	 * @param AKeyLen length of the key
	 * @return 0 on success, negative on failure
	 */
	static int hmacSha256InitFunc(void **AHmacContext, const uint8_t *AKey, size_t AKeyLen, void *AUserData);

	/**
	 * Callback for an HMAC-SHA256 implementation.
	 * This function shall update the HMAC context with the provided data
	 *
	 * @param AHmacContext private HMAC context pointer
	 * @param AData pointer to the data
	 * @param ADataLen length of the data
	 * @return 0 on success, negative on failure
	 */
	static int hmacSha256UpdateFunc(void *AHmacContext, const uint8_t *AData, size_t ADataLen, void *AUserData);

	/**
	 * Callback for an HMAC-SHA256 implementation.
	 * This function shall finalize an HMAC calculation and populate the output
	 * buffer with the result.
	 *
	 * @param AHmacContext private HMAC context pointer
	 * @param AOutput buffer to be allocated and populated with the result
	 * @return 0 on success, negative on failure
	 */
	static int hmacSha256FinalFunc(void *AHmacContext, signal_buffer **AOutput, void *AUserData);

	/**
	 * Callback for an HMAC-SHA256 implementation.
	 * This function shall free the private context allocated in
	 * hmac_sha256_init_func.
	 *
	 * @param AHmacContext private HMAC context pointer
	 */
	static void hmacSha256CleanupFunc(void *AHmacContext, void *AUserData);

	/**
	 * Callback for a SHA512 message digest implementation.
	 * This function shall initialize a digest context.
	 *
	 * @param ADigestContext private digest context pointer
	 * @return 0 on success, negative on failure
	 */
	static int sha512DigestInitFunc(void **ADigestContext, void *AUserData);

	/**
	 * Callback for a SHA512 message digest implementation.
	 * This function shall update the digest context with the provided data.
	 *
	 * @param ADigestContext private digest context pointer
	 * @param AData pointer to the data
	 * @param ADataLen length of the data
	 * @return 0 on success, negative on failure
	 */
	static int sha512DigestUpdateFunc(void *ADigestContext, const uint8_t *AData, size_t ADataLen, void *AUserData);

	/**
	 * Callback for a SHA512 message digest implementation.
	 * This function shall finalize the digest calculation, populate the
	 * output buffer with the result, and prepare the context for reuse.
	 *
	 * @param ADigestContext private digest context pointer
	 * @param AOutput buffer to be allocated and populated with the result
	 * @return 0 on success, negative on failure
	 */
	static int sha512DigestFinalFunc(void *ADigestContext, signal_buffer **AOutput, void *AUserData);

	/**
	 * Callback for a SHA512 message digest implementation.
	 * This function shall free the private context allocated in
	 * sha512_digest_init_func.
	 *
	 * @param ADigestContext private digest context pointer
	 */
	static void sha512DigestCleanupFunc(void *ADigestContext, void *AUserData);

	/**
	 * Callback for an AES encryption implementation.
	 *
	 * @param AOutput buffer to be allocated and populated with the ciphertext
	 * @param ACipher specific cipher variant to use, either SG_CIPHER_AES_CTR_NOPADDING or SG_CIPHER_AES_CBC_PKCS5
	 * @param AKey the encryption key
	 * @param AKeyLen length of the encryption key
	 * @param AIv the initialization vector
	 * @param AIvLen length of the initialization vector
	 * @param APlaintext the plaintext to encrypt
	 * @param APlaintextLen length of the plaintext
	 * @return 0 on success, negative on failure
	 */
	static int encryptFunc(signal_buffer **AOutput, int ACipher,
						   const uint8_t *AKey, size_t AKeyLen,
						   const uint8_t *AIv, size_t AIvLen,
						   const uint8_t *APlaintext, size_t APlaintextLen,
						   void *AUserData);

	/**
	 * Callback for an AES decryption implementation.
	 *
	 * @param AOutput buffer to be allocated and populated with the plaintext
	 * @param ACipher specific cipher variant to use, either SG_CIPHER_AES_CTR_NOPADDING or SG_CIPHER_AES_CBC_PKCS5
	 * @param AKey the encryption key
	 * @param AKeyLen length of the encryption key
	 * @param AIv the initialization vector
	 * @param AIvLen length of the initialization vector
	 * @param ACiphertext the ciphertext to decrypt
	 * @param ACiphertextLen length of the ciphertext
	 * @return 0 on success, negative on failure
	 */
	static int decryptFunc(signal_buffer **AOutput, int ACipher,
						   const uint8_t *AKey, size_t AKeyLen,
						   const uint8_t *AIv, size_t AIvLen,
						   const uint8_t *ACiphertext, size_t ACiphertextLen,
						   void *AUserData);

	static void recursiveMutexLock(void *AUserData);
	static void recursiveMutexUnlock(void *AUserData);

	void recursiveMutexLock();
	void recursiveMutexUnlock();

private:
	// signal-protocol
	signal_context		*FGlobalContext;
	signal_protocol_store_context * FStoreContext;
	IIdentityKeyListener *FIdentityKeyListener;
	quint32				FVersion;

	QString FConnectionName;

	// Mutex
	QMutex	*FMutex;

	// Error code
	int FError;
};

#endif // SIGNALPROTOCOL_H
