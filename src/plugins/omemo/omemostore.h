#ifndef OMEMOSTORE_H
#define OMEMOSTORE_H

#include <QSqlDatabase>

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

	void init(const QString &ADatabaseFileName);
	void uninit();

	/**
	 * Creates the necessary tables. Safe to call if they already exist.
	 *
	 * @return 0 on success, negative on error
	 */
	int axc_db_create();

	/**
	 * Drops all the tables so that the db can be reset.
	 *
	 * @return 0 on success, negative on error
	 */
	int axc_db_destroy();

	/**
	 * Sets the value of a property in the database's "settings" table.
	 *
	 * @param name The name of the property.
	 * @param status The int value of the property.
	 * @return 0 on success, negative on error
	 */
	int axc_db_property_set(const char * name, const int val);

	/**
	 * Gets a property from the settings table.
	 *
	 * @param name Name of the property
	 * @param val_p Pointer to where the saved value should be stored.
	 * @return 0 on success, negative on error, 1 if no sql error but no result
	 */
	int axc_db_property_get(const char * name, int * val_p);

	/**
	 * "Partial application" of db_set_property, setting the init status value.
	 *
	 * @param status AXC_DB_NOT INITIALIZED, AXC_DB_NEEDS_ROOLBACK, or AXC_DB_INITIALIZED
	 * @return 0 on success, negative on error
	 */
	int axc_db_init_status_set(const int status);

	/**
	 * "Partial application" of db_get_property, getting the init status value.
	 *
	 * @param init_status_p The value behind this pointer will be set to the init status number.
	 * @return 0 on success, negative on error, 1 if no sql error but no result
	 */
	int axc_db_init_status_get(int * init_status_p);

	/**
	 * Saves the public and private key by using the api serialization calls, as this format (and not the higher-level key type) is needed by the getter.
	 *
	 * @param Pointer to the keypair as returned by axolotl_key_helper_generate_identity_key_pair
	 * @return 0 on success, negative on error
	 */
	int axc_db_identity_set_key_pair(const ratchet_identity_key_pair * key_pair_p);

	/**
	 * Saves the axolotl registration ID which was obtained by a call to axolotl_key_helper_generate_registration_id().
	 *
	 * @param reg_id The ID.
	 * @return 0 on success, negative on error
	 */
	int axc_db_identity_set_local_registration_id(const uint32_t reg_id);

	/**
	 * Stores a whole list of pre keys at once, inside a single transaction.
	 *
	 * @param pre_keys_head Pointer to the first element of the list.
	 */
	int axc_db_pre_key_store_list(signal_protocol_key_helper_pre_key_list_node * pre_keys_head);

	// Session store methods

	int axc_db_session_load(signal_buffer ** record,
							signal_buffer ** user_record,
							const signal_protocol_address * address,
							void * user_data);
	int axc_db_session_get_sub_device_sessions(signal_int_list ** sessions,
											   const char * name, size_t name_len,
											   void * user_data);
	int axc_db_session_store(const signal_protocol_address *address,
							 uint8_t *record, size_t record_len,
							 uint8_t *user_record, size_t user_record_len,
							 void *user_data);
	int axc_db_session_contains(const signal_protocol_address * address,
								void * user_data);
	int axc_db_session_delete(const signal_protocol_address * address,
							  void * user_data);
	int axc_db_session_delete_all(const char * name, size_t name_len, void * user_data);
	void axc_db_session_destroy_store_ctx(void * user_data);

	// Pre key store methods
	int axc_db_pre_key_load(signal_buffer ** record, uint32_t pre_key_id, void * user_data);
	int axc_db_pre_key_store(uint32_t pre_key_id, uint8_t * record, size_t record_len, void * user_data);
	int axc_db_pre_key_contains(uint32_t pre_key_id, void * user_data);
	int axc_db_pre_key_remove(uint32_t pre_key_id, void * user_data);
	void axc_db_pre_key_destroy_ctx(void * user_data);

	// Signed pre key store methods
	int axc_db_signed_pre_key_load(signal_buffer ** record, uint32_t signed_pre_key_id, void * user_data);
	int axc_db_signed_pre_key_store(uint32_t signed_pre_key_id, uint8_t * record, size_t record_len, void * user_data);
	int axc_db_signed_pre_key_contains(uint32_t signed_pre_key_id, void * user_data);
	int axc_db_signed_pre_key_remove(uint32_t signed_pre_key_id, void * user_data);
	void axc_db_signed_pre_key_destroy_ctx(void * user_data);	

	// Identity key store implementation
	int axc_db_identity_get_key_pair(signal_buffer ** public_data, signal_buffer ** private_data, void * user_data);
	int axc_db_identity_get_local_registration_id(void * user_data, uint32_t * registration_id);
	int axc_db_identity_save(const signal_protocol_address * addr_p, uint8_t * key_data, size_t key_len, void * user_data);
	int axc_db_identity_always_trusted(const signal_protocol_address * addr_p, uint8_t * key_data, size_t key_len, void * user_data);
	void axc_db_identity_destroy_ctx(void * user_data);
};

#endif // OMEMOSTORE_H
