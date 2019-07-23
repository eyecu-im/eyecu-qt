#ifndef OMEMOSTORE_H
#define OMEMOSTORE_H

#include <QSqlDatabase>
#include <QMap>

class SignalProtocol;
struct signal_context;
struct signal_buffer;
struct signal_protocol_address;
struct signal_int_list;
struct signal_protocol_key_helper_pre_key_list_node;
struct ratchet_identity_key_pair;

namespace OmemoStore
{
	enum DbInitState {
		DbNotInitialized = -1,
		DbNeedsRollback,
		DbInitialized
	};

	void addDatabase(const QString &ADatabaseFileName, const QString &AConnectionName);
	void removeDatabase(const QString &AConnectionName);

	/**
	 * Creates the necessary tables. Safe to call if they already exist.
	 *
	 * @return 0 on success, negative on error
	 */
	int create(SignalProtocol *ASignalProtocol);

	/**
	 * Drops all the tables so that the db can be reset.
	 *
	 * @return 0 on success, negative on error
	 */
	int destroy(SignalProtocol *ASignalProtocol);

	/**
	 * Sets the value of a property in the database's "settings" table.
	 *
	 * @param AName The name of the property.
	 * @param AValue The int value of the property.
	 * @return 0 on success, negative on error
	 */
	int propertySet(const QString &AName, const int AValue, SignalProtocol *ASignalProtocol);

	/**
	 * Gets a property from the settings table.
	 *
	 * @param AName Name of the property
	 * @param AValue Pointer to where the saved value should be stored.
	 * @return 0 on success, negative on error, 1 if no sql error but no result
	 */
	int propertyGet(const QString &AName, int &AValue, SignalProtocol *ASignalProtocol);

	/**
	 * "Partial application" of db_set_property, setting the init status value.
	 *
	 * @param AStatus one of DbInitState values
	 * @param ASignalProtocol pointer to SignalProtocol object
	 * @return 0 on success, negative on error
	 */
	int initStatusSet(int AStatus, SignalProtocol *ASignalProtocol);

	/**
	 * "Partial application" of propertyGet(), getting the init status value.
	 *
	 * @param AInitStatus Will be set to the init status.
	 * @return 0 on success, negative on error, 1 if no SQL error but no result
	 */
	int initStatusGet(int &AInitStatus, SignalProtocol *ASignalProtocol);

	/**
	 * Stores a whole list of pre keys at once, inside a single transaction.
	 *
	 * @param APreKeysHead Pointer to the first element of the list.
	 */
	int preKeyStoreList(signal_protocol_key_helper_pre_key_list_node * APreKeysHead,
						SignalProtocol *ASignalProtocol);

	/**
	 * Gets the specified number of pre keys for publishing, i.e. only their public part.
	 *
	 * @param AAmount Number of keys to retrieve.
	 * @param APreKeys A map to be filled will deserialized public pre keys, associated with their IDs.
	 * @param AContext pointer to SignalProtocol global context.
	 * @return 0 on success, negative on error.
	 */
	int preKeyGetList(size_t AAmount, QMap<quint32, QByteArray> &APreKeys,
					  const SignalProtocol *ASignalProtocol);

	/**
	 * Retrieves the highest existing pre key ID that is not the last resort key's ID.
	 *
	 * @param AMaxId Will be set to the highest ID that is not MAX_INT.
	 * @return 0 on success, negative on error.
	 */
	int preKeyGetMaxId(uint32_t &AMaxId, SignalProtocol *ASignalProtocol);

	/**
	 * Returns the count of pre keys saved in the database.
	 * This includes the "last resort" key that is additionally generated at db init.
	 *
	 * @return pre key count, negative on error.
	 */
	int preKeyGetCount(SignalProtocol *ASignalProtocol);

	/**
	 * saves the public and private key by using the api serialization calls, as this format (and not the higher-level key type) is needed by the getter.
	 */
	int identitySetKeyPair(const ratchet_identity_key_pair * AKeyPair, SignalProtocol *ASignalProtocol);

	int identitySetLocalRegistrationId(SignalProtocol *ASignalProtocol, const uint32_t ARegistrationId);

	int identityIsTrusted(const QString &AName, const QByteArray &AKeyData, SignalProtocol *ASignalProtocol);

	//
	// Callback methods
	//

	// Session store methods

	int sessionLoad(signal_buffer ** ARecord,
					signal_buffer ** AUserRecord,
					const signal_protocol_address * AAddress,
					void * AUserData);
	int sessionGetSubDeviceSessions(signal_int_list ** ASessions,
									const char * AName, size_t ANameLen,
									void * AUserData);
	int sessionStore(const signal_protocol_address *AAddress,
					 uint8_t *ARecord, size_t ARecordLen,
					 uint8_t *AUserRecord, size_t AUserRecordLen,
					 void *AUserData);
	int sessionContains(const signal_protocol_address * AAddress, void * AUserData);
	int sessionDelete(const signal_protocol_address * AAddress, void * AUserData);
	int sessionDeleteAll(const char * AName, size_t ANameLen, void * AUserData);
	void sessionDestroyStoreCtx(void * AUserData);

	// Pre key store methods
	int preKeyLoad(signal_buffer ** ARecord, uint32_t APreKeyId, void * AUserData);
	int preKeyStore(uint32_t APreKeyId, uint8_t * ARecord,
					size_t ARecordLen, void * AUserData);
	int preKeyContains(uint32_t APreKey_Id, void * AUserData);
	int preKeyRemove(uint32_t APreKeyId, void * AUserData);
	void preKeyDestroyCtx(void * AUserData);

	// Signed pre key store methods
	int signedPreKeyLoad(signal_buffer ** ARecord, uint32_t ASignedPreKeyId, void * AUserData);
	int signedPreKeyStore(uint32_t ASignedPreKeyId, uint8_t * ARecord,
						  size_t ARecordLen, void * AUserData);
	int signedPreKeyContains(uint32_t ASignedPreKeyId, void * AUserData);
	int signedPreKeyRemove(uint32_t ASignedPreKeyId, void * AUserData);
	void signedPreKeyDestroyCtx(void * AUserData);

	// Identity key store implementation
	int identityGetKeyPair(signal_buffer ** APublicData, signal_buffer ** APrivateData, void * AUserData);
	int identityGetLocalRegistrationId(void * AUserData, uint32_t * ARegistrationId);
	int identitySave(const signal_protocol_address * AAddress, uint8_t * AKeyData, size_t AKeyLen, void * AUserData);
	int identityAlwaysTrusted(const signal_protocol_address * AAddress, uint8_t * AKeyData, size_t AKeyLen, void * AUserData);
	void identityDestroyCtx(void * AUserData);
};

#endif // OMEMOSTORE_H
