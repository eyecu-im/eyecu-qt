#ifndef OMEMOSTORE_H
#define OMEMOSTORE_H

#include <QSqlDatabase>

struct signal_buffer;
struct signal_protocol_address;
struct signal_int_list;

namespace OmemoStore
{
	void init(const QString &ADatabaseFileName);
	void uninit();

	// Session store static methods
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

};

#endif // OMEMOSTORE_H
