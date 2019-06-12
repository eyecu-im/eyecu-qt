#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>
#include <key_helper.h>
#include "signalprotocol.h"
#include "omemostore.h"

#define CONNECTION_NAME "OMEMO"

#define INIT_STATUS_NAME "init_status"
#define OWN_PUBLIC_KEY_NAME "own_public_key"
#define OWN_PRIVATE_KEY_NAME "own_private_key"
#define OWN_KEY 2
#define REG_ID_NAME "axolotl_registration_id"
#define IDENTITY_KEY_TRUSTED 1
#define IDENTITY_KEY_UNTRUSTED 1

#define SESSION_STORE_TABLE_NAME "session_store"
#define SESSION_STORE_NAME_NAME "name"
#define SESSION_STORE_NAME_LEN_NAME "name_len"
#define SESSION_STORE_DEVICE_ID_NAME "device_id"
#define SESSION_STORE_RECORD_NAME "session_record"
#define SESSION_STORE_RECORD_LEN_NAME "record_len"
#define PRE_KEY_STORE_TABLE_NAME "pre_key_store"
#define PRE_KEY_STORE_ID_NAME "id"
#define PRE_KEY_STORE_RECORD_NAME "pre_key_record"
#define PRE_KEY_STORE_RECORD_LEN_NAME "record_len"
#define SIGNED_PRE_KEY_STORE_TABLE_NAME "signed_pre_key_store"
#define SIGNED_PRE_KEY_STORE_ID_NAME "id"
#define SIGNED_PRE_KEY_STORE_RECORD_NAME "signed_pre_key_record"
#define SIGNED_PRE_KEY_STORE_RECORD_LEN_NAME "record_len"
#define IDENTITY_KEY_STORE_TABLE_NAME "identity_key_store"
#define IDENTITY_KEY_STORE_NAME_NAME "name"
#define IDENTITY_KEY_STORE_KEY_NAME "key"
#define IDENTITY_KEY_STORE_KEY_LEN_NAME "key_len"
#define IDENTITY_KEY_STORE_TRUSTED_NAME "trusted"
#define SETTINGS_STORE_TABLE_NAME "settings"
#define SETTINGS_STORE_NAME_NAME "name"
#define SETTINGS_STORE_PROPERTY_NAME "property"


/**
 * Convenience function for opening a db "connection" and at the same time preparing a statement.
 *
 * @param db_pp Will be set to the db connection pointer.
 * @param stmt The SQL statement.
 * @param user_data_p Optional. The user_data as received from the axolotl interface, will be used to set the database name.
 * @return 0 on success, negative on failure
 */
//static bool db_conn_open(QSqlDatabase db, const QString &stmt, QSqlQuery &query, void * AUserData) {
//	SignalProtocol * signalProtocol = reinterpret_cast<SignalProtocol*>(AUserData);

//	if (stmt.isEmpty()) {
//		qCritical("stmt is empty");
//		return false;
//	}

//	db.setDatabaseName(signalProtocol->dbFileName());
//	if (!db.open()) {
//		qCritical("Failed to open db_p");
//		return false;
//	}

//	QSqlQuery sql(db);

//	if (sql.prepare(stmt))
//		return sql;

//	qCritical("Failed to prepare statement");
//	return QSqlQuery();
//}

namespace OmemoStore
{
// Session store implementation
void init(const QString &ADatabaseFileName)
{
	QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", CONNECTION_NAME);
	db.setDatabaseName(ADatabaseFileName);
	db.open();
}

void uninit()
{
	QSqlDatabase::database(CONNECTION_NAME).close();
	QSqlDatabase::removeDatabase(CONNECTION_NAME);
}

QSqlDatabase db()
{
	return QSqlDatabase::database(CONNECTION_NAME);
}


int axc_db_property_set(const char * name, const int val) {
	// 1 - name of property
	// 2 - value
	const QString stmt("INSERT OR REPLACE INTO " SETTINGS_STORE_TABLE_NAME " VALUES (?1, ?2);");
	QSqlQuery pstmt_p(stmt, db());

	pstmt_p.bindValue(1, name);
	pstmt_p.bindValue(2, val);

	if (!pstmt_p.exec()) return -3;

	return 0;
}

int axc_db_property_get(const char * name, int * val_p) {
	const QString stmt("SELECT * FROM " SETTINGS_STORE_TABLE_NAME " WHERE name IS ?1;");

//	sqlite3 * db_p = (void *) 0;
//	sqlite3_stmt * pstmt_p = (void *) 0;
	QSqlQuery pstmt_p(stmt, db());
//	if (db_conn_open(&db_p, &pstmt_p, stmt, axc_ctx_p)) return -1;

	pstmt_p.bindValue(1, name);
//	if (sqlite3_bind_text(pstmt_p, 1, name, -1, SQLITE_STATIC)) {
//		db_conn_cleanup(db_p, pstmt_p, "Failed to bind", __func__, axc_ctx_p);
//		return -21;
//	}

	if (pstmt_p.exec())	{
		if (pstmt_p.next()) {
			qCritical("Result not found");
			return 1;
		} else {
			const int temp = pstmt_p.value(1).toInt();

			// exactly one result
			if (pstmt_p.next()) {
				qCritical("Too many results");
				return -3;
			}

			*val_p = temp;
			return 0;
		}
	} else {
		qCritical("Failed to execute statement");
		return -3;
	}
}



int axc_db_session_load(signal_buffer ** record,
						signal_buffer ** user_record,
						const signal_protocol_address * address,
						void * user_data)
{
	Q_UNUSED(user_data);
	Q_UNUSED(user_record);

	const QString stmt("SELECT * FROM " SESSION_STORE_TABLE_NAME
					   " WHERE " SESSION_STORE_NAME_NAME " IS ?1"
					   " AND " SESSION_STORE_DEVICE_ID_NAME " IS ?2;");

	QSqlQuery pstmt_p(stmt, db());

	pstmt_p.bindValue(QString(1), QString(address->name));
	pstmt_p.bindValue(QString(2), address->device_id);

	if (pstmt_p.exec()) {
		if (pstmt_p.isActive()) {
			if (pstmt_p.next()) {
				const int record_len = pstmt_p.value(4).toInt();
				*record = signal_buffer_create(
							reinterpret_cast<quint8*>(
								pstmt_p.value(3).toByteArray().data()),
							size_t(record_len));
				if (*record == nullptr) {
				  qCritical("Buffer could not be initialised");
				  return -3;
				}
			} else {
				// session not found
				return 0;
			}
		} else {
			qCritical("SQL query is in inactive state!");
			return -3;
		}
	} else {
		qCritical("Failed executing SQL statement");
		return -3;
	}

	return 1;
}

int axc_db_session_get_sub_device_sessions(signal_int_list ** sessions,
													   const char * name, size_t name_len,
													   void * user_data)
{
	Q_UNUSED(user_data);

	const QString stmt("SELECT * FROM " SESSION_STORE_TABLE_NAME " WHERE " SESSION_STORE_NAME_NAME " IS ?1;");

	QSqlQuery pstmt_p(stmt, db());

	signal_int_list * session_list_p = nullptr;
	int ret_val = 0;

	pstmt_p.bindValue(1, name);
	if (!pstmt_p.isValid()) {
		qCritical("Failed to bind name when trying to find sub device sessions");
		ret_val = -21;
		goto cleanup;
	}

	session_list_p = signal_int_list_alloc();

	if (pstmt_p.exec()) {
		while (pstmt_p.next())
			signal_int_list_push_back(session_list_p,
									  pstmt_p.value(2).toInt());
	} else {
		qCritical("Error while retrieving result rows");
		ret_val = -3;
		goto cleanup;
	}

	(void) name_len;

	*sessions = session_list_p;
	ret_val = int(signal_int_list_size(*sessions));

cleanup:
	if (ret_val < 0) {
		if (session_list_p) {
		  signal_int_list_free(session_list_p);
		}
	}

	return ret_val;
}

int axc_db_session_store(const signal_protocol_address *address,
						 uint8_t *record, size_t record_len,
						 uint8_t *user_record, size_t user_record_len,
						 void *user_data)
{
	Q_UNUSED(user_data);
	Q_UNUSED(user_record);
	Q_UNUSED(user_record_len);

	const QString stmt("INSERT OR REPLACE INTO " SESSION_STORE_TABLE_NAME " VALUES (:name, :name_len, :device_id, :session_record, :record_len);");

	QSqlQuery pstmt_p(stmt, db());

	pstmt_p.bindValue(":name", QString(address->name));
	pstmt_p.bindValue(":name_len", address->name_len);
	pstmt_p.bindValue(3, address->device_id);
	pstmt_p.bindValue(4, QByteArray(reinterpret_cast<char *>(record), int(record_len)));
	pstmt_p.bindValue(5, record_len);

	if (!pstmt_p.exec()) return -3;

	return 0;
}

int axc_db_session_contains(const signal_protocol_address *address, void *user_data)
{
	Q_UNUSED(user_data);

	const QString stmt(	"SELECT * FROM " SESSION_STORE_TABLE_NAME
						" WHERE " SESSION_STORE_NAME_NAME " IS ?1"
						" AND " SESSION_STORE_DEVICE_ID_NAME " IS ?2;");

	QSqlQuery pstmt_p(stmt, db());

	pstmt_p.bindValue(1, QString(address->name));
	pstmt_p.bindValue(2, address->device_id);

	if (pstmt_p.exec()) {
		if (pstmt_p.next())
			return 1;
		else
			return 0;
	} else {
		qCritical("Failed executing SQL statement");
		return -3;
	}
}

int axc_db_session_delete(const signal_protocol_address *address, void *user_data)
{
	Q_UNUSED(user_data);

	const QString stmt( "DELETE FROM " SESSION_STORE_TABLE_NAME
						" WHERE " SESSION_STORE_NAME_NAME " IS ?1"
						" AND " SESSION_STORE_DEVICE_ID_NAME " IS ?2;");

	QSqlQuery pstmt_p(stmt, db());

	pstmt_p.bindValue(1, QString(address->name));
	pstmt_p.bindValue(2, address->device_id);

	if (pstmt_p.exec()) {
		if (pstmt_p.numRowsAffected())
			return 1;
		else
			return 0;
	} else {
		qCritical("Failed to delete session");
		return -4;
	}
}

int axc_db_session_delete_all(const char *name, size_t name_len, void *user_data)
{
	Q_UNUSED(user_data);
	Q_UNUSED(name_len);

	const QString stmt("DELETE FROM " SESSION_STORE_TABLE_NAME " WHERE " SESSION_STORE_NAME_NAME " IS ?1;");

	QSqlQuery pstmt_p(stmt, db());

	pstmt_p.bindValue(1, QString(name));

	if (pstmt_p.exec())
		return pstmt_p.numRowsAffected();
	else {
		qCritical("Failed to delete sessions");
		return -4;
	}
}

void axc_db_session_destroy_store_ctx(void *user_data)
{
	Q_UNUSED(user_data);
}

// pre key store impl
int axc_db_pre_key_load(signal_buffer ** record, uint32_t pre_key_id, void * user_data)
{
	Q_UNUSED(user_data);

	const QString stmt("SELECT * FROM " PRE_KEY_STORE_TABLE_NAME " WHERE " PRE_KEY_STORE_ID_NAME " IS ?1;");
	QSqlQuery pstmt_p(stmt, db());

	pstmt_p.bindValue(1, pre_key_id);

	if (pstmt_p.exec()) {
		if (pstmt_p.next()) {
			const int record_len = pstmt_p.value(2).toInt();
			*record = signal_buffer_create(
						reinterpret_cast<quint8*>(
							pstmt_p.value(1).toByteArray().data()),
						uint(record_len));
			if (!*record) {
				qCritical("Buffer could not be initialised");
				return -3;
			}
		} else {
			// session not found
			return SG_ERR_INVALID_KEY_ID;
		}
	} else {
		qCritical("Failed executing SQL statement");
		return -3;
	}

	return SG_SUCCESS;
}

int axc_db_pre_key_store(uint32_t pre_key_id, uint8_t * record, size_t record_len, void * user_data)
{
	Q_UNUSED(user_data);

	const QString stmt("INSERT OR REPLACE INTO " PRE_KEY_STORE_TABLE_NAME " VALUES (?1, ?2, ?3);");
	QSqlQuery pstmt_p(stmt, db());

	pstmt_p.bindValue(1, pre_key_id);
	pstmt_p.bindValue(2, QByteArray(reinterpret_cast<char*>(record), int(record_len)));
	pstmt_p.bindValue(3, record_len);

	if (!pstmt_p.exec()) return -3;

	return 0;
}

int axc_db_pre_key_store_list(signal_protocol_key_helper_pre_key_list_node * pre_keys_head,
							  SignalProtocol * sp_ctx_p)
{
	Q_UNUSED(sp_ctx_p);

	const QString stmt_begin("BEGIN TRANSACTION;");
	const QString stmt("INSERT OR REPLACE INTO " PRE_KEY_STORE_TABLE_NAME " VALUES (?1, ?2, ?3);");

	signal_buffer * key_buf_p = nullptr;
	signal_protocol_key_helper_pre_key_list_node * pre_keys_curr_p = nullptr;
	session_pre_key * pre_key_p = nullptr;

	QSqlQuery pstmt_p(stmt_begin, db());

	if (!pstmt_p.exec()) {
		qCritical("Failed to execute statement");
		return -3;
	}
	pstmt_p.finish();

	if (!pstmt_p.prepare(stmt)) {
		qCritical("Failed to prepare statement");
		return -2;
	}

	pre_keys_curr_p = pre_keys_head;
	while (pre_keys_curr_p) {
		pre_key_p = signal_protocol_key_helper_key_list_element(pre_keys_curr_p);
		if (session_pre_key_serialize(&key_buf_p, pre_key_p)) {
			qCritical("Failed to serialize pre key");
			return -1;
		}

		pstmt_p.bindValue(1, session_pre_key_get_id(pre_key_p));
		pstmt_p.bindValue(2, QByteArray(reinterpret_cast<char*>(signal_buffer_data(key_buf_p)),
										int(signal_buffer_len(key_buf_p))));
		pstmt_p.bindValue(3, signal_buffer_len(key_buf_p));
		if (!pstmt_p.exec()) {
			qCritical("Failed to execute statement");
			return -3;
		}

		signal_buffer_bzero_free(key_buf_p);

//TODO: Check, if we need these
		pstmt_p.finish();
		pstmt_p.clear();

		pre_keys_curr_p = signal_protocol_key_helper_key_list_next(pre_keys_curr_p);
	}

	return 0;
}

int axc_db_pre_key_get_list(size_t amount, SignalProtocol * axc_ctx_p,
							QList<axc_buf_list_item> &list_head_pp) {
	const QString stmt("SELECT * FROM " PRE_KEY_STORE_TABLE_NAME
					   " ORDER BY " PRE_KEY_STORE_ID_NAME " ASC LIMIT ?1;");

	int ret_val = -1;
	char * err_msg = nullptr;

	quint32 key_id = 0;
	signal_buffer * serialized_keypair_data_p = nullptr;
	size_t record_len = 0;
	session_pre_key * pre_key_p = nullptr;
	ec_key_pair * pre_key_pair_p = nullptr;
	ec_public_key * pre_key_public_p = nullptr;
	signal_buffer * pre_key_public_serialized_p = nullptr;

	QSqlQuery pstmt_p(stmt, db());

	pstmt_p.bindValue(1, amount);

	if (pstmt_p.exec())
	{
		while (pstmt_p.next())
		{
			key_id = pstmt_p.value(0).toUInt();
			record_len = pstmt_p.value(2).toUInt();
			serialized_keypair_data_p = signal_buffer_create(
						reinterpret_cast<quint8 *>(pstmt_p.value(1).toByteArray().data()),
						record_len);
			if (!serialized_keypair_data_p) {
				err_msg = "failed to initialize buffer";
				ret_val = -3;
				goto cleanup;
			}

			ret_val = session_pre_key_deserialize(&pre_key_p,
												  signal_buffer_data(serialized_keypair_data_p),
												  record_len,
												  axc_ctx_p->globalContext());
			if (ret_val) {
				err_msg = "failed to deserialize keypair";
				goto cleanup;
			}

			pre_key_pair_p = session_pre_key_get_key_pair(pre_key_p);
			pre_key_public_p = ec_key_pair_get_public(pre_key_pair_p);

			ret_val = ec_public_key_serialize(&pre_key_public_serialized_p, pre_key_public_p);
			if (ret_val) {
				err_msg = "failed to serialize public key";
				goto cleanup;
			}

			axc_buf_list_item temp_item_p(key_id, pre_key_public_serialized_p);

			list_head_pp.append(temp_item_p);

			signal_buffer_bzero_free(serialized_keypair_data_p);

			SIGNAL_UNREF(pre_key_p);
			pre_key_p = nullptr;
		}
	} else {
		err_msg = "sql error when retrieving keys";
		goto cleanup;
	}

	ret_val = 0;

cleanup:
	if (ret_val)
	{
		signal_buffer_bzero_free(serialized_keypair_data_p);
		SIGNAL_UNREF(pre_key_p);
		signal_buffer_bzero_free(pre_key_public_serialized_p);
		list_head_pp.clear();
	}
	if (err_msg)
		qCritical(err_msg);
	return ret_val;
}

int axc_db_pre_key_contains(uint32_t pre_key_id, void * user_data)
{
	Q_UNUSED(user_data);

	const QString stmt("SELECT * FROM " PRE_KEY_STORE_TABLE_NAME " WHERE " PRE_KEY_STORE_ID_NAME " IS ?1;");

	QSqlQuery pstmt_p(stmt, db());

	pstmt_p.bindValue(1, pre_key_id);

	if (pstmt_p.exec()) {
		if (pstmt_p.next())
			return 1;
		else
			return 0;
	} else {
		qCritical("Failed executing SQL statement");
		return -3;
	}
}

int axc_db_pre_key_get_max_id(uint32_t * max_id_p)
{
	const QString stmt("SELECT MAX(" PRE_KEY_STORE_ID_NAME ") FROM " PRE_KEY_STORE_TABLE_NAME
					   " WHERE " PRE_KEY_STORE_ID_NAME " IS NOT ("
					   "   SELECT MAX(" PRE_KEY_STORE_ID_NAME ") FROM " PRE_KEY_STORE_TABLE_NAME
					   " );");

	char * err_msg = nullptr;
	int ret_val = 0;
	uint32_t id = 0;

	QSqlQuery pstmt_p(stmt, db());

	if (pstmt_p.exec()) {
		if (pstmt_p.next()) {
			id = pstmt_p.value(3).toUInt();
			if (!id) {
				err_msg = "db not initialized";
				ret_val = -1;
			} else {
				*max_id_p = id;
				ret_val = 0;
			}
		} else {
			err_msg = "No value returned";
			ret_val = -4;
		}
	} else {
		err_msg = "Failed executing SQL statement";
		ret_val = -3;
	}

	if (err_msg)
		qCritical(err_msg);

	return ret_val;
}

int axc_db_pre_key_get_count(size_t * count_p) {
	const QString stmt("SELECT count(" PRE_KEY_STORE_ID_NAME") FROM " PRE_KEY_STORE_TABLE_NAME ";");

	QSqlQuery pstmt_p(stmt, db());

	if (pstmt_p.exec())
	{
		if (pstmt_p.next()) {
			*count_p = pstmt_p.value(0).toUInt();
			return 0;
		} else {
			qCritical("count returned an error");
			return -1;
		}
	} else {
		qCritical("Failed executing SQL statement");
		return -3;
	}
}

int axc_db_pre_key_remove(uint32_t pre_key_id, void * user_data)
{
	Q_UNUSED(user_data);

	const QString stmt("DELETE FROM " PRE_KEY_STORE_TABLE_NAME " WHERE " PRE_KEY_STORE_ID_NAME " IS ?1;");

	QSqlQuery pstmt_p(stmt, db());

	pstmt_p.bindValue(1, pre_key_id);

	if (pstmt_p.exec()) {
		if (pstmt_p.numRowsAffected()) {
			return 0;
		} else {
			qCritical("Key does not exist");
			return -4;
		}
	} else {
		qCritical("Failed to delete session");
		return -4;
	}
}

void axc_db_pre_key_destroy_ctx(void * user_data)
{
  Q_UNUSED(user_data);
  //const char stmt[] = "DELETE FROM pre_key_store; VACUUM;";

  //db_exec_quick(stmt, user_data);
}

// signed pre key store impl
int axc_db_signed_pre_key_load(signal_buffer ** record, uint32_t signed_pre_key_id, void * user_data)
{
	Q_UNUSED(user_data);

	const QString stmt("SELECT * FROM " SIGNED_PRE_KEY_STORE_TABLE_NAME " WHERE " SIGNED_PRE_KEY_STORE_ID_NAME " IS ?1;");
	QSqlQuery pstmt_p(stmt, db());

	pstmt_p.bindValue(1, signed_pre_key_id);

	if (pstmt_p.exec()) {
		if (pstmt_p.next()) {
			const uint record_len = pstmt_p.value(2).toUInt();
			*record = signal_buffer_create(
						reinterpret_cast<quint8 *>(pstmt_p.value(1).toByteArray().data()), record_len);
			if (!*record) {
				qCritical("Buffer could not be initialised");
				return -3;
			}
		} else {
			// session not found
			return SG_ERR_INVALID_KEY_ID;
		}
	} else {
		qCritical("Failed executing SQL statement");
		return -3;
	}

	return SG_SUCCESS;
}

int axc_db_signed_pre_key_store(uint32_t signed_pre_key_id, uint8_t * record, size_t record_len, void * user_data)
{
	Q_UNUSED(user_data);

	const QString stmt("INSERT OR REPLACE INTO " SIGNED_PRE_KEY_STORE_TABLE_NAME " VALUES (?1, ?2, ?3);");
	QSqlQuery pstmt_p(stmt, db());

	pstmt_p.bindValue(1, signed_pre_key_id);
	pstmt_p.bindValue(2, QByteArray(reinterpret_cast<char*>(record), int(record_len)));
	pstmt_p.bindValue(3, record_len);

	if (!pstmt_p.exec()) return -3;

	return 0;
}

int axc_db_signed_pre_key_contains(uint32_t signed_pre_key_id, void * user_data)
{
	Q_UNUSED(user_data);

	const QString stmt("SELECT * FROM " SIGNED_PRE_KEY_STORE_TABLE_NAME " WHERE " SIGNED_PRE_KEY_STORE_ID_NAME " IS ?1;");
	QSqlQuery pstmt_p(stmt, db());

	pstmt_p.bindValue(1, signed_pre_key_id);

	if (pstmt_p.exec())	{
		if (pstmt_p.next()) // result exists
			return 1;
		else // no result
			return 0;
	} else {
		qCritical("Failed executing SQL statement");
		return -3;
	}
}

int axc_db_signed_pre_key_remove(uint32_t signed_pre_key_id, void * user_data)
{
	Q_UNUSED(user_data);

	const QString stmt("DELETE FROM " SIGNED_PRE_KEY_STORE_TABLE_NAME " WHERE " SIGNED_PRE_KEY_STORE_ID_NAME " IS ?1;");
	QSqlQuery pstmt_p(stmt, db());

	pstmt_p.bindValue(1, signed_pre_key_id);

	if (pstmt_p.exec())	{
		if (pstmt_p.numRowsAffected())
			return 0;
		else {
			qCritical("Key does not exist");
			return -4;
		}
	} else {
		qCritical("Failed to delete session");
		return -4;
	}
}

void axc_db_signed_pre_key_destroy_ctx(void * user_data)
{
	Q_UNUSED(user_data);
//	const char stmt[] = "DELETE FROM signed_pre_key_store; VACUUM;";

//	db_exec_quick(stmt, user_data);
}


// Identity key store implementation
/**
 * saves the public and private key by using the api serialization calls, as this format (and not the higher-level key type) is needed by the getter.
 */
int axc_db_identity_set_key_pair(const ratchet_identity_key_pair * key_pair_p, axc_context * axc_ctx_p) {
	// 1 - name ("public" or "private")
	// 2 - key blob
	// 3 - length of the key
	// 4 - trusted (1 for true, 0 for false)
	const QString stmt("INSERT INTO " IDENTITY_KEY_STORE_TABLE_NAME " VALUES (?1, ?2, ?3, ?4);");

//  sqlite3 * db_p = (void *) 0;
//  sqlite3_stmt * pstmt_p = (void *) 0;

  char * err_msg = nullptr;
  int ret_val = 0;
  signal_buffer * pubkey_buf_p = nullptr;
  signal_buffer * privkey_buf_p = nullptr;
  size_t pubkey_buf_len = 0;
  uint8_t * pubkey_buf_data_p = nullptr;
  size_t privkey_buf_len = 0;
  uint8_t * privkey_buf_data_p = nullptr;

  QSqlQuery pstmt_p(stmt, db());
//	if (db_conn_open(&db_p, &pstmt_p, stmt, axc_ctx_p)) return -1;

	// public key
	pstmt_p.bindValue(1, OWN_PUBLIC_KEY_NAME);
//	if (sqlite3_bind_text(pstmt_p, 1, OWN_PUBLIC_KEY_NAME, -1, SQLITE_STATIC)) {
//		err_msg = "Failed to bind";
//		ret_val = -21;
//		goto cleanup;
//	}

	if (ec_public_key_serialize(&pubkey_buf_p, ratchet_identity_key_pair_get_public(key_pair_p))) {
		err_msg = "Failed to allocate memory to serialize the public key";
		ret_val = SG_ERR_NOMEM;
		goto cleanup;
	}
	pubkey_buf_len = signal_buffer_len(pubkey_buf_p);
	pubkey_buf_data_p = signal_buffer_data(pubkey_buf_p);

	pstmt_p.bindValue(2, QByteArray(reinterpret_cast<char*>(pubkey_buf_data_p),
									int(pubkey_buf_len)));
//	if (sqlite3_bind_blob(pstmt_p, 2, pubkey_buf_data_p, pubkey_buf_len, SQLITE_TRANSIENT)) {
//		err_msg = "Failed to bind";
//		ret_val = -22;
//		goto cleanup;
//	}
	pstmt_p.bindValue(3, pubkey_buf_len);
//	if(sqlite3_bind_int(pstmt_p, 3, pubkey_buf_len)) {
//		err_msg = "Failed to bind";
//		ret_val = -23;
//		goto cleanup;
//	}

	pstmt_p.bindValue(4, OWN_KEY);
//	if (sqlite3_bind_int(pstmt_p, 4, OWN_KEY)) {
//		err_msg = "Failed to bind";
//		ret_val = -24;
//		goto cleanup;
//	}
	if (pstmt_p.exec()) {
//	if (sqlite3_step(pstmt_p) != SQLITE_DONE) {
		err_msg = "Failed to execute statement";
		ret_val = -3;
		goto cleanup;
	}
	if (pstmt_p.numRowsAffected() != 1) {
//	if (sqlite3_changes(db_p) != 1) {
		err_msg = "Failed to insert";
		ret_val = -3;
		goto cleanup;
	}

	// private key
//TODO: Check, if we really need this
		pstmt_p.finish();
//	if (sqlite3_reset(pstmt_p)) {
//		err_msg = "Failed to reset prepared statement";
//		ret_val = -2;
//		goto cleanup;
//	}
//TODO: Check, if we really need this
		pstmt_p.clear();
//		sqlite3_clear_bindings(pstmt_p);

		pstmt_p.bindValue(1, OWN_PRIVATE_KEY_NAME);
//	if (sqlite3_bind_text(pstmt_p, 1, OWN_PRIVATE_KEY_NAME, -1, SQLITE_STATIC)) {
//		err_msg = "Failed to bind";
//		ret_val = -21;
//		goto cleanup;
//	}

	if (ec_private_key_serialize(&privkey_buf_p, ratchet_identity_key_pair_get_private(key_pair_p))) {
		err_msg = "Failed to allocate memory to serialize the private key";
		ret_val = SG_ERR_NOMEM;
		goto cleanup;
	}
	privkey_buf_len = signal_buffer_len(privkey_buf_p);
	privkey_buf_data_p = signal_buffer_data(privkey_buf_p);

	pstmt_p.bindValue(2, QByteArray(reinterpret_cast<char*>(privkey_buf_data_p),
									int(privkey_buf_len)));
//	if (sqlite3_bind_blob(pstmt_p, 2, privkey_buf_data_p, privkey_buf_len, SQLITE_TRANSIENT)) {
//		err_msg = "Failed to bind";
//		ret_val = -22;
//		goto cleanup;
//	}

	pstmt_p.bindValue(3, privkey_buf_len);
//	if(sqlite3_bind_int(pstmt_p, 3, privkey_buf_len)) {
//		err_msg = "Failed to bind";
//		ret_val = -23;
//		goto cleanup;
//	}

//TODO: Check, if we really need this
	pstmt_p.bindValue(4, OWN_KEY);
//	if (sqlite3_bind_int(pstmt_p, 4, OWN_KEY)) {
//		err_msg = "Failed to bind";
//		ret_val = -24;
//		goto cleanup;
//	}
	if (pstmt_p.exec()) {
//	if (sqlite3_step(pstmt_p) != SQLITE_DONE) {
		err_msg = "Failed to execute statement";
		ret_val = -3;
		goto cleanup;
	}

	if (pstmt_p.numRowsAffected() != 1) {
//	if (sqlite3_changes(db_p) != 1) {
		err_msg = "Failed to insert";
		ret_val = -3;
		goto cleanup;
	}

cleanup:
	if (pubkey_buf_p) {
		signal_buffer_bzero_free(pubkey_buf_p);
	}

	if (privkey_buf_p) {
		signal_buffer_bzero_free(privkey_buf_p);
	}

	qCritical(err_msg);
//	db_conn_cleanup(db_p, pstmt_p, err_msg, __func__, axc_ctx_p);
	return ret_val;
}


int axc_db_identity_get_key_pair(signal_buffer ** public_data, signal_buffer ** private_data, void * user_data)
{
	Q_UNUSED(user_data);

	const QString stmt("SELECT * FROM " IDENTITY_KEY_STORE_TABLE_NAME " WHERE " IDENTITY_KEY_STORE_NAME_NAME " IS ?1;");
//	axc_context * axc_ctx_p = (axc_context *) user_data;
//	sqlite3 * db_p = (void *) 0;
//	sqlite3_stmt * pstmt_p = (void *) 0;
	QSqlQuery pstmt_p(stmt, db());
//	if (db_conn_open(&db_p, &pstmt_p, stmt, user_data)) return -1;

	char * err_msg = nullptr;
	int ret_val = 0;
	signal_buffer * pubkey_buf_p = nullptr;
	signal_buffer * privkey_buf_p = nullptr;

  // public key
	pstmt_p.bindValue(1, OWN_PUBLIC_KEY_NAME);
//	if (sqlite3_bind_text(pstmt_p, 1, OWN_PUBLIC_KEY_NAME, -1, SQLITE_STATIC)) {
//		err_msg = "Failed to bind public key name when trying to get the identity key pair";
//		ret_val = -21;
//		goto cleanup;
//	}

	size_t pubkey_len = 0;
	size_t privkey_len = 0;
//	int step_result = sqlite3_step(pstmt_p);
	if (pstmt_p.exec()) {
		if (pstmt_p.next()) {
			pubkey_len = pstmt_p.value(2).toUInt();
			pubkey_buf_p = signal_buffer_create(
						reinterpret_cast<uint8_t*>(pstmt_p.value(1).toByteArray().data()),
						pubkey_len);

			if (!pubkey_buf_p) {
				err_msg = "Buffer could not be initialised";
				ret_val = -3;
				goto cleanup;
			}
		} else {
			// public key not found
			err_msg = "Own public key not found";
			ret_val = SG_ERR_INVALID_KEY_ID;
			goto cleanup;
		}
//	if (step_result == SQLITE_DONE) {
		// public key not found
//		err_msg = "Own public key not found";
//		ret_val = SG_ERR_INVALID_KEY_ID;
//		goto cleanup;
//	} else if (step_result == SQLITE_ROW) {
//		pubkey_len = sqlite3_column_int(pstmt_p, 2);
//		pubkey_buf_p = signal_buffer_create(sqlite3_column_blob(pstmt_p, 1), pubkey_len);

//		if (pubkey_buf_p == 0) {
//			err_msg = "Buffer could not be initialised";
//			ret_val = -3;
//			goto cleanup;
//		}
	} else {
		err_msg = "Failed executing SQL statement";
		ret_val = -3;
		goto cleanup;
	}

	pstmt_p.finish();
//	sqlite3_reset(pstmt_p);
	pstmt_p.clear();
//	sqlite3_clear_bindings(pstmt_p);

	// private key
	pstmt_p.bindValue(1, OWN_PRIVATE_KEY_NAME);
//	if (sqlite3_bind_text(pstmt_p, 1, OWN_PRIVATE_KEY_NAME, -1, SQLITE_STATIC)) {
//		err_msg = "Failed to bind private key name when trying to get the identity key pair";
//		ret_val = -21;
//		goto cleanup;
//	}

	if (pstmt_p.exec()) {
		if (pstmt_p.next()) {
			privkey_len = pstmt_p.value(2).toUInt();
			privkey_buf_p = signal_buffer_create(
						reinterpret_cast<uint8_t*>(
							pstmt_p.value(1).toByteArray().data()),
						privkey_len);

			if (!privkey_buf_p) {
				err_msg = "Buffer could not be initialised";
				ret_val = -3;
				goto cleanup;
			}
		} else {
			// private key not found
			err_msg = "Own private key not found";
			ret_val = SG_ERR_INVALID_KEY_ID;
			goto cleanup;
		}
//	step_result = sqlite3_step(pstmt_p);
//	if (step_result == SQLITE_DONE) {
//		// private key not found
//		err_msg = "Own private key not found";
//		ret_val = SG_ERR_INVALID_KEY_ID;
//		goto cleanup;
//	} else if (step_result == SQLITE_ROW) {
//		privkey_len = sqlite3_column_int(pstmt_p, 2);
//		privkey_buf_p = signal_buffer_create(sqlite3_column_blob(pstmt_p, 1), privkey_len);

//		if (privkey_buf_p == 0) {
//			err_msg = "Buffer could not be initialised";
//			ret_val = -3;
//			goto cleanup;
//		}
	} else {
		err_msg = "Failed executing SQL statement";
		ret_val = -3;
		goto cleanup;
	}

	*public_data = pubkey_buf_p;
	*private_data = privkey_buf_p;

cleanup:
	if (ret_val < 0) {
		if (pubkey_buf_p) {
			signal_buffer_bzero_free(pubkey_buf_p);
		}
		if (privkey_buf_p) {
			signal_buffer_bzero_free(privkey_buf_p);
		}
	}

	qCritical(err_msg);
//	db_conn_cleanup(db_p, pstmt_p, err_msg, __func__, axc_ctx_p);
	return ret_val;
}

int axc_db_identity_set_local_registration_id(const uint32_t reg_id, SignalProtocol * axc_ctx_p)
{
	return (axc_db_property_set(REG_ID_NAME, int(reg_id))) ? -1 : 0;
}

int axc_db_identity_get_local_registration_id(void * user_data, uint32_t * registration_id) {
	const QString stmt("SELECT * FROM " SETTINGS_STORE_TABLE_NAME " WHERE " SETTINGS_STORE_NAME_NAME " IS ?1;");

//	axc_context * axc_ctx_p = (axc_context *) user_data;
//	sqlite3 * db_p = (void *) 0;
//	sqlite3_stmt * pstmt_p = (void *) 0;
	QSqlQuery pstmt_p(stmt, db());
//	if (db_conn_open(&db_p, &pstmt_p, stmt, user_data)) return -1;
	pstmt_p.bindValue(1, REG_ID_NAME);
//	if (sqlite3_bind_text(pstmt_p, 1, REG_ID_NAME, -1, SQLITE_STATIC)) {
//		db_conn_cleanup(db_p, pstmt_p, "Failed to bind", __func__, axc_ctx_p);
//		return -21;
//	}

//	const int step_result = sqlite3_step(pstmt_p);

	if (pstmt_p.exec()) {
		if (pstmt_p.next()) {
			*registration_id = pstmt_p.value(1).toUInt();
		} else {
			qCritical("Own registration ID not found");
			return -31;
		}
	} else {
		qCritical("Failed executing SQL statement");
		return -32;
	}

//	if (step_result == SQLITE_DONE) {
//		// registration ID not found
//		db_conn_cleanup(db_p, pstmt_p, "Own registration ID not found", __func__, axc_ctx_p);
//		return -31;
//	} else if (step_result == SQLITE_ROW) {
//		*registration_id = sqlite3_column_int(pstmt_p, 1);
//	} else {
//		db_conn_cleanup(db_p, pstmt_p, "Failed executing SQL statement", __func__, axc_ctx_p);
//		return -32;
//	}

//	db_conn_cleanup(db_p, pstmt_p, (void *) 0, __func__, axc_ctx_p);
	return 0;
}

int axc_db_identity_save(const signal_protocol_address * addr_p, uint8_t * key_data, size_t key_len, void * user_data) {
  // 1 - name ("public" or "private" for own keys, name for contacts)
  // 2 - key blob
  // 3 - length of the key
  // 4 - trusted (1 for true, 0 for false)
  char save_stmt[] = "INSERT OR REPLACE INTO " IDENTITY_KEY_STORE_TABLE_NAME " VALUES (?1, ?2, ?3, ?4);";
  char del_stmt[] = "DELETE FROM " IDENTITY_KEY_STORE_TABLE_NAME " WHERE " IDENTITY_KEY_STORE_NAME_NAME " IS ?1;";
  char * stmt = (void *) 0;

  if (key_data) {
	stmt = save_stmt;
  } else {
	stmt = del_stmt;
  }

  axc_context * axc_ctx_p = (axc_context *) user_data;
  sqlite3 * db_p = (void *) 0;
  sqlite3_stmt * pstmt_p = (void *) 0;
  if (db_conn_open(&db_p, &pstmt_p, stmt, user_data)) return -1;

  if (sqlite3_bind_text(pstmt_p, 1, addr_p->name, -1, SQLITE_TRANSIENT)) {
	db_conn_cleanup(db_p, pstmt_p, "Failed to bind", __func__, axc_ctx_p);
	return -21;
  }

  if (key_data) {
	if (sqlite3_bind_blob(pstmt_p, 2, key_data, key_len, SQLITE_TRANSIENT)) {
	  db_conn_cleanup(db_p, pstmt_p, "Failed to bind", __func__, axc_ctx_p);
	  return -22;
	}
	if(sqlite3_bind_int(pstmt_p, 3, key_len)) {
	  db_conn_cleanup(db_p, pstmt_p, "Failed to bind", __func__, axc_ctx_p);
	  return -23;
	}
	if(sqlite3_bind_int(pstmt_p, 4, IDENTITY_KEY_TRUSTED)) {
	  db_conn_cleanup(db_p, pstmt_p, "Failed to bind", __func__, axc_ctx_p);
	  return -24;
	}
  }

  if (db_exec_single_change(db_p, pstmt_p, axc_ctx_p)) return -3;

  db_conn_cleanup(db_p, pstmt_p, (void *) 0, __func__, axc_ctx_p);
  return 0;
}

int axc_db_identity_is_trusted(const char * name, size_t name_len, uint8_t * key_data, size_t key_len, void * user_data) {
	const QString stmt("SELECT * FROM " IDENTITY_KEY_STORE_TABLE_NAME " WHERE " IDENTITY_KEY_STORE_NAME_NAME " IS ?1;");

//	axc_context * axc_ctx_p = (axc_context *) user_data;
//	sqlite3 * db_p = (void *) 0;
//	sqlite3_stmt * pstmt_p = (void *) 0;
	signal_buffer * key_record = nullptr;
	int step_result = 0;
	size_t record_len = 0;

	if (db_conn_open(&db_p, &pstmt_p, stmt, user_data)) return -1;

	if (sqlite3_bind_text(pstmt_p, 1, name, -1, SQLITE_TRANSIENT)) {
	db_conn_cleanup(db_p, pstmt_p, "Failed to bind", __func__, axc_ctx_p);
	return -21;
	}

	step_result = sqlite3_step(pstmt_p);
	if (step_result == SQLITE_DONE) {
	// no entry = trusted, according to docs
	db_conn_cleanup(db_p, pstmt_p, (void *) 0, __func__, axc_ctx_p);
	return 1;
	} else if (step_result == SQLITE_ROW) {
	// theoretically could be checked if trusted or not but it's TOFU

	record_len = sqlite3_column_int(pstmt_p, 2);
	if (record_len != key_len) {
	  db_conn_cleanup(db_p, pstmt_p, "Key length does not match", __func__, axc_ctx_p);
	  return 0;
	}

	key_record = signal_buffer_create(sqlite3_column_blob(pstmt_p, 1), record_len);
	if (key_record == 0) {
	  db_conn_cleanup(db_p, pstmt_p, "Buffer could not be initialised", __func__, axc_ctx_p);
	  return -3;
	}

	if (memcmp(key_data, signal_buffer_data(key_record), key_len)) {
	  db_conn_cleanup(db_p, pstmt_p, "Key data does not match", __func__, axc_ctx_p);
	}

	db_conn_cleanup(db_p, pstmt_p, (void *) 0, __func__, axc_ctx_p);
	signal_buffer_bzero_free(key_record);
	return 1;
	} else {
	db_conn_cleanup(db_p, pstmt_p, "Failed executing SQL statement", __func__, axc_ctx_p);
	return -32;
	}

	(void)name_len;
}

int axc_db_identity_always_trusted(const signal_protocol_address * addr_p, uint8_t * key_data, size_t key_len, void * user_data) {
  (void) addr_p;
  (void) key_data;
  (void) key_len;
  (void) user_data;

  return 1;
}

void axc_db_identity_destroy_ctx(void * user_data) {
  (void) user_data;
  //const char stmt[] = "DELETE FROM identity_key_store; VACUUM;";

  //db_exec_quick(stmt, user_data);
}


}
