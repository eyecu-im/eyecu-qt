#ifndef OMEMOSTORE_H
#define OMEMOSTORE_H

#include <QSqlDatabase>
#include <QMap>

struct signal_context;
struct signal_buffer;
struct signal_protocol_address;
struct signal_int_list;
struct signal_protocol_key_helper_pre_key_list_node;
struct ratchet_identity_key_pair;

namespace OmemoStore
{
	#define AXC_DB_NOT_INITIALIZED (-1)
	#define AXC_DB_NEEDS_ROLLBACK    0
	#define AXC_DB_INITIALIZED       1

	void addDatabase(const QString &ADatabaseFileName, const QString &AConnectionName);
	void removeDatabase(const QString &AConnectionName);

	/**
	 * Creates the necessary tables. Safe to call if they already exist.
	 *
	 * @return 0 on success, negative on error
	 */
	int create(void *AUserData);

	/**
	 * Drops all the tables so that the db can be reset.
	 *
	 * @return 0 on success, negative on error
	 */
	int destroy(void *AUserData);

	/**
	 * Sets the value of a property in the database's "settings" table.
	 *
	 * @param name The name of the property.
	 * @param status The int value of the property.
	 * @return 0 on success, negative on error
	 */
	int propertySet(const char * name, const int val, void *AUserData);

	/**
	 * Gets a property from the settings table.
	 *
	 * @param name Name of the property
	 * @param val_p Pointer to where the saved value should be stored.
	 * @return 0 on success, negative on error, 1 if no sql error but no result
	 */
	int propertyGet(const char * name, int * val_p, void *AUserData);

	/**
	 * "Partial application" of db_set_property, setting the init status value.
	 *
	 * @param status AXC_DB_NOT INITIALIZED, AXC_DB_NEEDS_ROOLBACK, or AXC_DB_INITIALIZED
	 * @return 0 on success, negative on error
	 */
	int initStatusSet(const int status, void *AUserData);

	/**
	 * "Partial application" of db_get_property, getting the init status value.
	 *
	 * @param init_status_p The value behind this pointer will be set to the init status number.
	 * @return 0 on success, negative on error, 1 if no sql error but no result
	 */
	int initStatusGet(int * init_status_p, void *AUserData);

	/**
	 * Saves the public and private key by using the api serialization calls, as this format (and not the higher-level key type) is needed by the getter.
	 *
	 * @param Pointer to the keypair as returned by axolotl_key_helper_generate_identity_key_pair
	 * @return 0 on success, negative on error
	 */
	int identitySetKeyPair(const ratchet_identity_key_pair * key_pair_p, void *AUserData);

	/**
	 * Saves the axolotl registration ID which was obtained by a call to axolotl_key_helper_generate_registration_id().
	 *
	 * @param ARegistrationId The ID.
	 * @return 0 on success, negative on error
	 */
	int identitySetLocalRegistrationId(void *AUserData, const uint32_t ARegistrationId);

	/**
	 * Stores a whole list of pre keys at once, inside a single transaction.
	 *
	 * @param APreKeysHead Pointer to the first element of the list.
	 */
	int preKeyStoreList(signal_protocol_key_helper_pre_key_list_node * APreKeysHead, void *AUserData);

	/**
	 * Gets the specified number of pre keys for publishing, i.e. only their public part.
	 *
	 * @param AAmount Number of keys to retrieve.
	 * @param APreKeys A map to be filled will deserialized public pre keys, associated with their IDs.
	 * @param AContext pointer to SignalProtocol global context.
	 * @return 0 on success, negative on error.
	 */
	int preKeyGetList(size_t AAmount, QMap<quint32, QByteArray> &APreKeys, const void *AUserData);

	/**
	 * Retrieves the highest existing pre key ID that is not the last resort key's ID.
	 *
	 * @param max_id_p Will be set to the highest ID that is not MAX_INT.
	 * @return 0 on success, negative on error.
	 */
	int preKeyGetMaxId(uint32_t &AMaxId, void *AUserData);

	/**
	 * Returns the count of pre keys saved in the database.
	 * This includes the "last resort" key that is additionally generated at db init.
	 *
	 * @return pre key count, negative on error.
	 */
	int preKeyGetCount(void *AUserData);

	// Session store methods

	int sessionLoad(signal_buffer ** record,
					signal_buffer ** user_record,
					const signal_protocol_address * address,
					void * AUserData);
	int sessionGetSubDeviceSessions(signal_int_list ** sessions,
									const char * name, size_t name_len,
									void * AUserData);
	int sessionStore(const signal_protocol_address *address,
					 uint8_t *record, size_t record_len,
					 uint8_t *user_record, size_t user_record_len,
					 void *AUserData);
	int sessionContains(const signal_protocol_address * address,
						void * AUserData);
	int sessionDelete(const signal_protocol_address * address, void * AUserData);
	int sessionDeleteAll(const char * name, size_t name_len, void * AUserData);
	void sessionDestroyStoreCtx(void * user_data);

	// Pre key store methods
	int preKeyLoad(signal_buffer ** record, uint32_t pre_key_id, void * AUserData);
	int preKeyStore(uint32_t pre_key_id, uint8_t * record,
					size_t record_len, void * AUserData);
	int preKeyContains(uint32_t pre_key_id, void * AUserData);
	int preKeyRemove(uint32_t pre_key_id, void * AUserData);
	void preKeyDestroyCtx(void * AUserData);

	// Signed pre key store methods
	int signedPreKeyLoad(signal_buffer ** record, uint32_t signed_pre_key_id, void * AUserData);
	int signedPreKeyStore(uint32_t signed_pre_key_id, uint8_t * record,
						  size_t record_len, void * AUserData);
	int signedPreKeyContains(uint32_t signed_pre_key_id, void * user_data);
	int signedPreKeyRemove(uint32_t signed_pre_key_id, void * AUserData);
	void signedPreKeyDestroyCtx(void * AUserData);

	// Identity key store implementation
	int identityGetKeyPair(signal_buffer ** public_data, signal_buffer ** private_data, void * AUserData);
	int identityGetLocalRegistrationId(void * AUserData, uint32_t * registration_id);
	int identitySave(const signal_protocol_address * addr_p, uint8_t * key_data, size_t key_len, void * AUserData);
	int identityAlwaysTrusted(const signal_protocol_address * addr_p, uint8_t * key_data, size_t key_len, void * user_data);
	void identityDestroyCtx(void * user_data);
};

#endif // OMEMOSTORE_H
