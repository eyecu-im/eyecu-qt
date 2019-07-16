#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
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
#define SESSION_STORE_DEVICE_ID_NAME "device_id"
#define SESSION_STORE_RECORD_NAME "session_record"
#define PRE_KEY_STORE_TABLE_NAME "pre_key_store"
#define PRE_KEY_STORE_ID_NAME "id"
#define PRE_KEY_STORE_RECORD_NAME "pre_key_record"
#define SIGNED_PRE_KEY_STORE_TABLE_NAME "signed_pre_key_store"
#define SIGNED_PRE_KEY_STORE_ID_NAME "id"
#define SIGNED_PRE_KEY_STORE_RECORD_NAME "signed_pre_key_record"
#define IDENTITY_KEY_STORE_TABLE_NAME "identity_key_store"
#define IDENTITY_KEY_STORE_NAME_NAME "name"
#define IDENTITY_KEY_STORE_KEY_NAME "key"
#define IDENTITY_KEY_STORE_TRUSTED_NAME "trusted"
#define SETTINGS_STORE_TABLE_NAME "settings"
#define SETTINGS_STORE_NAME_NAME "name"
#define SETTINGS_STORE_PROPERTY_NAME "property"

#define ADDR_NAME(X) QString::fromLatin1(QByteArray(X->name, int(X->name_len)))

namespace OmemoStore
{
// Session store implementation
void init(const QString &ADatabaseFileName)
{
	qDebug("OmemoStore::init(\"%s\")", ADatabaseFileName.toUtf8().data());
	QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", CONNECTION_NAME);
	db.setDatabaseName(ADatabaseFileName);
	if (db.open())
		qDebug("Database opened!");
	else
		qCritical("Database open failed!");
}

void uninit()
{
	qDebug("OmemoStore::uninit()");
	QSqlDatabase::database(CONNECTION_NAME).close();
	QSqlDatabase::removeDatabase(CONNECTION_NAME);
}

QSqlDatabase db()
{
	qDebug("OmemoStore::db()");
	return QSqlDatabase::database(CONNECTION_NAME);
}

int propertySet(const char * name, const int val)
{
	qDebug("OmemoStore::propertySet(%s, %d)", name, val);
	// 1 - name of property
	// 2 - value
	const QString stmt("INSERT OR REPLACE INTO " SETTINGS_STORE_TABLE_NAME " VALUES (?1, ?2)");
	QSqlQuery pstmt_p(stmt, db());

	pstmt_p.bindValue(0, name);
	pstmt_p.bindValue(1, val);

	if (!pstmt_p.exec()) return -3;

	return 0;
}

int propertyGet(const char * name, int * val_p)
{
	qDebug("OmemoStore::propertyGet(%s, %p)", name, (void *)val_p);
	const QString stmt("SELECT * FROM " SETTINGS_STORE_TABLE_NAME " WHERE name IS ?1");

	QSqlQuery pstmt_p(stmt, db());
	pstmt_p.bindValue(0, name);

	if (pstmt_p.exec())	{
		if (pstmt_p.next()) {
			const int temp = pstmt_p.value(1).toInt();
			// exactly one result
			if (pstmt_p.next()) {
				qCritical("Too many results");
				return -3;
			}
			*val_p = temp;
			return 0;
		} else {
			qCritical("Result not found");
			return 1;
		}
	} else
		return -3;
}

int sessionLoad(signal_buffer ** record,
				signal_buffer ** user_record,
				const signal_protocol_address * address,
				void * user_data)
{
	Q_UNUSED(user_data);
	Q_UNUSED(user_record);

	const QString stmt("SELECT * FROM " SESSION_STORE_TABLE_NAME
					   " WHERE " SESSION_STORE_NAME_NAME " IS ?1"
					   " AND " SESSION_STORE_DEVICE_ID_NAME " IS ?2");

	QSqlQuery pstmt_p(stmt, db());

	pstmt_p.bindValue(0, ADDR_NAME(address));
	pstmt_p.bindValue(1, address->device_id);

	if (pstmt_p.exec()) {
		if (pstmt_p.next()) {
			QByteArray data = pstmt_p.value(2).toByteArray();
			*record = signal_buffer_create(
						reinterpret_cast<quint8*>(data.data()),
						size_t(data.size()));
			if (*record == nullptr) {
			  qCritical("Buffer could not be initialised");
			  return -3;
			}
		} else {
			// session not found
			return 0;
		}
	} else {
		qCritical("Failed executing SQL statement");
		return -3;
	}

	return 1;
}

int sessionGetSubDeviceSessions(signal_int_list ** sessions,
								const char * name, size_t name_len,
								void * user_data)
{
	Q_UNUSED(user_data);
	Q_UNUSED(name_len);

	const QString stmt("SELECT * FROM " SESSION_STORE_TABLE_NAME
					   " WHERE " SESSION_STORE_NAME_NAME " IS ?1");

	QSqlQuery pstmt_p(stmt, db());

	signal_int_list * session_list_p = nullptr;
	int ret_val = 0;

	pstmt_p.bindValue(0, name);
	if (!pstmt_p.isValid()) {
		qCritical("Failed to bind name when trying to find sub device sessions");
		ret_val = -21;
		goto cleanup;
	}

	session_list_p = signal_int_list_alloc();

	if (pstmt_p.exec()) {
		while (pstmt_p.next())
			signal_int_list_push_back(session_list_p, pstmt_p.value(2).toInt());
	} else {
		qCritical("Error while retrieving result rows");
		ret_val = -3;
		goto cleanup;
	}

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

int sessionStore(const signal_protocol_address *address,
				 uint8_t *record, size_t record_len,
				 uint8_t *user_record, size_t user_record_len,
				 void *user_data)
{
	Q_UNUSED(user_data);
	Q_UNUSED(user_record);
	Q_UNUSED(user_record_len);

	const QString stmt("INSERT OR REPLACE INTO " SESSION_STORE_TABLE_NAME
					   " VALUES (:name, :device_id, :session_record)");

	QSqlQuery pstmt_p(stmt, db());

	pstmt_p.bindValue(0, ADDR_NAME(address));
	pstmt_p.bindValue(1, address->device_id);
	pstmt_p.bindValue(2, QByteArray(reinterpret_cast<char *>(record),
									int(record_len)));
	if (!pstmt_p.exec())
	{
		qCritical("QSL statement execution failed: \"%s\"; rc=%d (%s)",
					pstmt_p.lastQuery().toUtf8().data(),
					pstmt_p.lastError().number(),
					pstmt_p.lastError().text().toUtf8().data());
		return -3;
	}

	return 0;
}

int sessionContains(const signal_protocol_address *address, void *user_data)
{
	Q_UNUSED(user_data);

	const QString stmt(	"SELECT * FROM " SESSION_STORE_TABLE_NAME
						" WHERE " SESSION_STORE_NAME_NAME " IS ?1"
						" AND " SESSION_STORE_DEVICE_ID_NAME " IS ?2");

	QSqlQuery pstmt_p(stmt, db());

	pstmt_p.bindValue(0, ADDR_NAME(address));
	pstmt_p.bindValue(1, address->device_id);

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

int sessionDelete(const signal_protocol_address *address, void *user_data)
{
	Q_UNUSED(user_data);

	const QString stmt( "DELETE FROM " SESSION_STORE_TABLE_NAME
						" WHERE " SESSION_STORE_NAME_NAME " IS ?1"
						" AND " SESSION_STORE_DEVICE_ID_NAME " IS ?2");

	QSqlQuery pstmt_p(stmt, db());

	pstmt_p.bindValue(0, ADDR_NAME(address));
	pstmt_p.bindValue(1, address->device_id);

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

int sessionDeleteAll(const char *name, size_t name_len, void *user_data)
{
	Q_UNUSED(user_data);
	Q_UNUSED(name_len);

	const QString stmt("DELETE FROM " SESSION_STORE_TABLE_NAME
					   " WHERE " SESSION_STORE_NAME_NAME " IS ?1");

	QSqlQuery pstmt_p(stmt, db());
	pstmt_p.bindValue(0, QString(name));

	if (pstmt_p.exec())
		return pstmt_p.numRowsAffected();
	else {
		qCritical("Failed to delete sessions");
		return -4;
	}
}

void sessionDestroyStoreCtx(void *user_data)
{
	Q_UNUSED(user_data);
}

// pre key store impl
int preKeyLoad(signal_buffer ** record, uint32_t pre_key_id, void * user_data)
{
	Q_UNUSED(user_data);

	const QString stmt("SELECT * FROM " PRE_KEY_STORE_TABLE_NAME
					   " WHERE " PRE_KEY_STORE_ID_NAME " IS ?1");
	QSqlQuery pstmt_p(stmt, db());

	pstmt_p.bindValue(0, pre_key_id);

	if (pstmt_p.exec()) {
		if (pstmt_p.next()) {
			QByteArray data = pstmt_p.value(1).toByteArray();
			*record = signal_buffer_create(
						reinterpret_cast<quint8*>(data.data()),
						size_t(data.size()));
			if (!*record) {
				qCritical("Buffer could not be initialised");
				return -3;
			}
		} else {
			qCritical("Pre key ID not found");
			return SG_ERR_INVALID_KEY_ID;
		}
	} else {
		qCritical("Failed executing SQL statement");
		return -3;
	}

	return SG_SUCCESS;
}

int preKeyStore(uint32_t pre_key_id, uint8_t * record, size_t record_len, void * user_data)
{
	Q_UNUSED(user_data);

	const QString stmt("INSERT OR REPLACE INTO " PRE_KEY_STORE_TABLE_NAME
					   " VALUES (?1, ?2)");
	QSqlQuery pstmt_p(stmt, db());

	pstmt_p.bindValue(0, pre_key_id);
	pstmt_p.bindValue(1, QByteArray(reinterpret_cast<char*>(record),
									int(record_len)));
	if (!pstmt_p.exec())
	{
		qCritical("Statement execution failed: \"%s\"; error=%d (%s)",
				  pstmt_p.lastQuery().toUtf8().data(),
				  pstmt_p.lastError().number(),
				  pstmt_p.lastError().text().toUtf8().data());
		return -3;
	}
	return 0;
}
/*
int axc_db_pre_key_get_list(size_t amount, SignalProtocol * axc_ctx_p,
							QList<axc_buf_list_item> &list_head_pp)
{
	const QString stmt("SELECT * FROM " PRE_KEY_STORE_TABLE_NAME
					   " ORDER BY " PRE_KEY_STORE_ID_NAME " ASC LIMIT ?1");

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

	pstmt_p.bindValue(0, amount);

	if (pstmt_p.exec())
	{
		while (pstmt_p.next())
		{
			key_id = pstmt_p.value(0).toUInt();
			QByteArray data = pstmt_p.value(1).toByteArray();
			serialized_keypair_data_p = signal_buffer_create(
						reinterpret_cast<quint8 *>(data.data()),
						size_t(data.size()));
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
*/
int preKeyContains(uint32_t pre_key_id, void * user_data)
{
	Q_UNUSED(user_data);

	const QString stmt("SELECT * FROM " PRE_KEY_STORE_TABLE_NAME
					   " WHERE " PRE_KEY_STORE_ID_NAME " IS ?1");

	QSqlQuery pstmt_p(stmt, db());

	pstmt_p.bindValue(0, pre_key_id);

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

/*
int axc_db_pre_key_get_max_id(uint32_t * max_id_p)
{
	const QString stmt("SELECT MAX(" PRE_KEY_STORE_ID_NAME ") FROM " PRE_KEY_STORE_TABLE_NAME
					   " WHERE " PRE_KEY_STORE_ID_NAME " IS NOT ("
					   "   SELECT MAX(" PRE_KEY_STORE_ID_NAME ") FROM " PRE_KEY_STORE_TABLE_NAME
					   " )");

	char * err_msg = nullptr;
	int ret_val = 0;
	uint32_t id = 0;

	QSqlQuery pstmt_p(stmt, db());

	if (pstmt_p.exec()) {
		if (pstmt_p.next()) {
			id = pstmt_p.value(0).toUInt();
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
	const QString stmt("SELECT count(" PRE_KEY_STORE_ID_NAME
					   ") FROM " PRE_KEY_STORE_TABLE_NAME);

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
*/
int preKeyRemove(uint32_t pre_key_id, void * user_data)
{
	Q_UNUSED(user_data);

	const QString stmt("DELETE FROM " PRE_KEY_STORE_TABLE_NAME
					   " WHERE " PRE_KEY_STORE_ID_NAME " IS ?1");

	QSqlQuery pstmt_p(stmt, db());

	pstmt_p.bindValue(0, pre_key_id);

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

void preKeyDestroyCtx(void * user_data)
{
  Q_UNUSED(user_data);
  //const char stmt[] = "DELETE FROM pre_key_store; VACUUM;";

  //db_exec_quick(stmt, user_data);
}

// signed pre key store impl
int signedPreKeyLoad(signal_buffer ** record, uint32_t signed_pre_key_id, void * user_data)
{
	Q_UNUSED(user_data);

	const QString stmt("SELECT * FROM " SIGNED_PRE_KEY_STORE_TABLE_NAME
					   " WHERE " SIGNED_PRE_KEY_STORE_ID_NAME " IS ?1");
	QSqlQuery pstmt_p(stmt, db());

	pstmt_p.bindValue(0, signed_pre_key_id);

	if (pstmt_p.exec()) {
		if (pstmt_p.next()) {
			QByteArray data = pstmt_p.value(1).toByteArray();
			*record = signal_buffer_create(reinterpret_cast<quint8 *>(data.data()),
										   uint(data.size()));
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

int signedPreKeyStore(uint32_t signed_pre_key_id, uint8_t * record, size_t record_len, void * user_data)
{
	Q_UNUSED(user_data);

	const QString stmt("INSERT OR REPLACE INTO " SIGNED_PRE_KEY_STORE_TABLE_NAME
					   " VALUES (?1, ?2)");
	QSqlQuery pstmt_p(stmt, db());

	pstmt_p.bindValue(0, signed_pre_key_id);
	pstmt_p.bindValue(1, QByteArray(reinterpret_cast<char*>(record), int(record_len)));

	if (!pstmt_p.exec()) return -3;

	return 0;
}

int signedPreKeyContains(uint32_t signed_pre_key_id, void * user_data)
{
	Q_UNUSED(user_data);

	const QString stmt("SELECT * FROM " SIGNED_PRE_KEY_STORE_TABLE_NAME
					   " WHERE " SIGNED_PRE_KEY_STORE_ID_NAME " IS ?1");
	QSqlQuery pstmt_p(stmt, db());

	pstmt_p.bindValue(0, signed_pre_key_id);

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

int signedPreKeyRemove(uint32_t signed_pre_key_id, void * user_data)
{
	Q_UNUSED(user_data);

	const QString stmt("DELETE FROM " SIGNED_PRE_KEY_STORE_TABLE_NAME
					   " WHERE " SIGNED_PRE_KEY_STORE_ID_NAME " IS ?1");
	QSqlQuery pstmt_p(stmt, db());

	pstmt_p.bindValue(0, signed_pre_key_id);

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

void signedPreKeyDestroyCtx(void * user_data)
{
	Q_UNUSED(user_data);
//	const char stmt[] = "DELETE FROM signed_pre_key_store; VACUUM;";

//	db_exec_quick(stmt, user_data);
}


// Identity key store implementation
/**
 * saves the public and private key by using the api serialization calls, as this format (and not the higher-level key type) is needed by the getter.
 */
int identitySetKeyPair(const ratchet_identity_key_pair * key_pair_p) {
	// 1 - name ("public" or "private")
	// 2 - key blob
	// 3 - trusted (1 for true, 0 for false)
	const QString stmt("INSERT INTO " IDENTITY_KEY_STORE_TABLE_NAME " VALUES (?1, ?2, ?3)");

	char * err_msg = nullptr;
	int ret_val = 0;
	signal_buffer * pubkey_buf_p = nullptr;
	signal_buffer * privkey_buf_p = nullptr;
//	size_t pubkey_buf_len = 0;
//	uint8_t * pubkey_buf_data_p = nullptr;
//	size_t privkey_buf_len = 0;
//	uint8_t * privkey_buf_data_p = nullptr;

	QSqlQuery pstmt_p(stmt, db());

	// public key
	pstmt_p.bindValue(0, OWN_PUBLIC_KEY_NAME);

	if (ec_public_key_serialize(&pubkey_buf_p,
								ratchet_identity_key_pair_get_public(key_pair_p))) {
		err_msg = "Failed to allocate memory to serialize the public key";
		ret_val = SG_ERR_NOMEM;
		goto cleanup;
	}

	pstmt_p.bindValue(1, QByteArray(reinterpret_cast<char*>(signal_buffer_data(pubkey_buf_p)),
									int(signal_buffer_len(pubkey_buf_p))));
	pstmt_p.bindValue(2, OWN_KEY);
	if (!pstmt_p.exec()) {
		err_msg = "Failed to execute statement";
		ret_val = -3;
		goto cleanup;
	}

	if (pstmt_p.numRowsAffected() != 1) {
		err_msg = "Failed to insert";
		ret_val = -3;
		goto cleanup;
	}

	// private key
//TODO: Check, if we really need this
//	pstmt_p.finish();

	pstmt_p.bindValue(0, OWN_PRIVATE_KEY_NAME);

	if (ec_private_key_serialize(&privkey_buf_p, ratchet_identity_key_pair_get_private(key_pair_p))) {
		err_msg = "Failed to allocate memory to serialize the private key";
		ret_val = SG_ERR_NOMEM;
		goto cleanup;
	}

	pstmt_p.bindValue(1, QByteArray(reinterpret_cast<char*>(signal_buffer_data(privkey_buf_p)),
									int(signal_buffer_len(privkey_buf_p))));
//TODO: Check, if we really need this
//	pstmt_p.bindValue(2, OWN_KEY);
	if (!pstmt_p.exec()) {
		err_msg = "Failed to execute statement";
		ret_val = -3;
		goto cleanup;
	}

	if (pstmt_p.numRowsAffected() != 1) {
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

	if (err_msg)
		qCritical(err_msg);

	return ret_val;
}

int identityGetKeyPair(signal_buffer ** public_data, signal_buffer ** private_data, void * user_data)
{
	Q_UNUSED(user_data);

	const QString stmt("SELECT * FROM " IDENTITY_KEY_STORE_TABLE_NAME
					   " WHERE " IDENTITY_KEY_STORE_NAME_NAME " IS ?1");

	QSqlQuery pstmt_p(stmt, db());

	char * err_msg = nullptr;
	int ret_val = 0;
	signal_buffer * pubkey_buf_p = nullptr;
	signal_buffer * privkey_buf_p = nullptr;

	// public key
	pstmt_p.bindValue(0, OWN_PUBLIC_KEY_NAME);
	if (pstmt_p.exec()) {
		if (pstmt_p.next()) {
			QByteArray data = pstmt_p.value(1).toByteArray();
			pubkey_buf_p = signal_buffer_create(
						reinterpret_cast<uint8_t*>(data.data()), size_t(data.size()));
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
	} else {
		err_msg = "Failed executing SQL statement";
		ret_val = -3;
		goto cleanup;
	}
//TODO: Check, if we need this
	pstmt_p.finish();

	// private key
	pstmt_p.bindValue(0, OWN_PRIVATE_KEY_NAME);

	if (pstmt_p.exec()) {
		if (pstmt_p.next()) {
			QByteArray data = pstmt_p.value(1).toByteArray();
			privkey_buf_p = signal_buffer_create(
						reinterpret_cast<uint8_t*>(data.data()), size_t(data.size()));

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

	if (err_msg)
		qCritical(err_msg);
	return ret_val;
}

int identitySetLocalRegistrationId(const uint32_t reg_id, SignalProtocol * axc_ctx_p)
{
	return (propertySet(REG_ID_NAME, int(reg_id))) ? -1 : 0;
}

int identityGetLocalRegistrationId(void * user_data, uint32_t * registration_id)
{
	Q_UNUSED(user_data);

	const QString stmt("SELECT * FROM " SETTINGS_STORE_TABLE_NAME
					   " WHERE " SETTINGS_STORE_NAME_NAME " IS ?1");
	QSqlQuery pstmt_p(stmt, db());

	pstmt_p.bindValue(0, REG_ID_NAME);

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

	return 0;
}

int identitySave(const signal_protocol_address * addr_p, uint8_t * key_data,
				 size_t key_len, void * user_data)
{
	Q_UNUSED(user_data);

	// 1 - name ("public" or "private" for own keys, name for contacts)
	// 2 - key blob
	// 3 - trusted (1 for true, 0 for false)
	const QString save_stmt("INSERT OR REPLACE INTO " IDENTITY_KEY_STORE_TABLE_NAME
							" VALUES (?1, ?2, ?3)");
	const QString del_stmt("DELETE FROM " IDENTITY_KEY_STORE_TABLE_NAME
						   " WHERE " IDENTITY_KEY_STORE_NAME_NAME " IS ?1");
	const QString &stmt = key_data?save_stmt:del_stmt;

	QSqlQuery pstmt_p(stmt, db());

	pstmt_p.bindValue(0, QString(addr_p->name));

	if (key_data) {
		pstmt_p.bindValue(1, QByteArray(
							  reinterpret_cast<char*>(key_data),
							  int(key_len)));
		pstmt_p.bindValue(2, IDENTITY_KEY_TRUSTED);
	}

	if (!pstmt_p.exec()) return -3;

	return 0;
}

int identityIsTrusted(const char * name, size_t name_len, uint8_t * key_data,
					  size_t key_len, void * user_data)
{
	Q_UNUSED(name_len);
	Q_UNUSED(user_data);

	const QString stmt("SELECT * FROM " IDENTITY_KEY_STORE_TABLE_NAME
					   " WHERE " IDENTITY_KEY_STORE_NAME_NAME " IS ?1");

	signal_buffer * key_record = nullptr;
	QSqlQuery pstmt_p(stmt, db());

	pstmt_p.bindValue(0, QString(name));

	if (pstmt_p.exec()) {
		if (pstmt_p.next()) {
			// no entry = trusted, according to docs
			return 1;
		} else {
			// theoretically could be checked if trusted or not but it's TOFU
			QByteArray data = pstmt_p.value(1).toByteArray();
			if (data.size() != int(key_len)) {
				qCritical("Key length does not match");
				return 0;
			}

			key_record = signal_buffer_create(
						reinterpret_cast<quint8*>(data.data()), size_t(data.size()));
			if (!key_record) {
				qCritical("Buffer could not be initialised");
				return -3;
			}

			if (memcmp(key_data, signal_buffer_data(key_record), key_len)) {
				qCritical("Key data does not match");
			}

			signal_buffer_bzero_free(key_record);
			return 1;
		}
	} else {
		qCritical("Failed executing SQL statement");
		return -32;
	}
}

int identityAlwaysTrusted(const signal_protocol_address * addr_p, uint8_t * key_data,
						  size_t key_len, void * user_data)
{
  Q_UNUSED(addr_p);
  Q_UNUSED(key_data);
  Q_UNUSED(key_len);
  Q_UNUSED(user_data);

  return 1;
}

void identityDestroyCtx(void * user_data)
{
  Q_UNUSED(user_data);
  //const char stmt[] = "DELETE FROM identity_key_store; VACUUM;";

	//db_exec_quick(stmt, user_data);
}

int create()
{
	qDebug() << "OmemoStore::create()";

	const QStringList stmts(
		QStringList() << "CREATE TABLE IF NOT EXISTS " SESSION_STORE_TABLE_NAME "("
							 SESSION_STORE_NAME_NAME " TEXT NOT NULL, "
							 SESSION_STORE_DEVICE_ID_NAME " INTEGER NOT NULL, "
							 SESSION_STORE_RECORD_NAME " BLOB NOT NULL, "
						 "  PRIMARY KEY(" SESSION_STORE_NAME_NAME ", " SESSION_STORE_DEVICE_ID_NAME "))"
					  << "CREATE TABLE IF NOT EXISTS " PRE_KEY_STORE_TABLE_NAME "("
							 PRE_KEY_STORE_ID_NAME " INTEGER NOT NULL PRIMARY KEY, "
							 PRE_KEY_STORE_RECORD_NAME " BLOB NOT NULL)"
					  << "CREATE TABLE IF NOT EXISTS " SIGNED_PRE_KEY_STORE_TABLE_NAME "("
							 SIGNED_PRE_KEY_STORE_ID_NAME " INTEGER NOT NULL PRIMARY KEY, "
							 SIGNED_PRE_KEY_STORE_RECORD_NAME " BLOB NOT NULL)"
					  << "CREATE TABLE IF NOT EXISTS " IDENTITY_KEY_STORE_TABLE_NAME "("
							 IDENTITY_KEY_STORE_NAME_NAME " TEXT NOT NULL PRIMARY KEY, "
							 IDENTITY_KEY_STORE_KEY_NAME " BLOB NOT NULL, "
							 IDENTITY_KEY_STORE_TRUSTED_NAME " INTEGER NOT NULL)"
					  << "CREATE TABLE IF NOT EXISTS " SETTINGS_STORE_TABLE_NAME "("
							 SETTINGS_STORE_NAME_NAME " TEXT NOT NULL PRIMARY KEY, "
							 SETTINGS_STORE_PROPERTY_NAME " INTEGER NOT NULL)");

	if (!db().transaction()) {
		qCritical("Failed to start transaction");
		return -3;
	}

	QSqlQuery pstmt_p(db());
	for (QStringList::ConstIterator it = stmts.constBegin();
		 it != stmts.constEnd(); ++it) {
		pstmt_p.prepare(*it);
		if (!pstmt_p.exec()) {
			db().rollback();
			return -1;
		}
	}

	if (db().commit())
		return 0;
	else
		return -2;
}

/**
 * Drops all tables.
 *
 * @param axc_ctx_p Pointer to the axc context.
 */
int destroy() {
	const QStringList stmts(
		QStringList() << "DROP TABLE IF EXISTS " SESSION_STORE_TABLE_NAME
					  << "DROP TABLE IF EXISTS " PRE_KEY_STORE_TABLE_NAME
					  << "DROP TABLE IF EXISTS " SIGNED_PRE_KEY_STORE_TABLE_NAME
					  << "DROP TABLE IF EXISTS " IDENTITY_KEY_STORE_TABLE_NAME
					  << "DROP TABLE IF EXISTS " SETTINGS_STORE_TABLE_NAME);

	if (!db().transaction()) {
		qCritical("Failed to start transaction");
		return -3;
	}

	QSqlQuery pstmt_p(db());
	for (QStringList::ConstIterator it = stmts.cbegin();
		 it != stmts.constEnd(); ++it) {
		pstmt_p.prepare(*it);
		if (!pstmt_p.exec()) {
			db().rollback();
			return -1;
		}
	}

	if (db().commit())
		return 0;
	else
		return -2;
}

int initStatusSet(const int status) {
	return propertySet(INIT_STATUS_NAME, status);
}

int initStatusGet(int *init_status_p)
{
	return propertyGet(INIT_STATUS_NAME, init_status_p);
}

int identitySetLocalRegistrationId(const uint32_t reg_id)
{
	return propertySet(REG_ID_NAME, int(reg_id)) ? -1 : 0;
}

int preKeyStoreList(signal_protocol_key_helper_pre_key_list_node *pre_keys_head)
{
	const QString stmt("INSERT OR REPLACE INTO " PRE_KEY_STORE_TABLE_NAME
					   " VALUES (?1, ?2)");
	signal_buffer * key_buf_p = nullptr;
	signal_protocol_key_helper_pre_key_list_node * pre_keys_curr_p = nullptr;
	session_pre_key * pre_key_p = nullptr;

	if (!db().transaction()) {
		qCritical("Failed to start transaction");
		return -3;
	}

	QSqlQuery pstmt_p(stmt, db());

	pre_keys_curr_p = pre_keys_head;
	while (pre_keys_curr_p) {
		pre_key_p = signal_protocol_key_helper_key_list_element(pre_keys_curr_p);
		if (session_pre_key_serialize(&key_buf_p, pre_key_p)) {
			qCritical("failed to serialize pre key");
			db().rollback();
			return -1;
		}

		pstmt_p.bindValue(0, session_pre_key_get_id(pre_key_p));
		pstmt_p.bindValue(1, QByteArray(reinterpret_cast<char*>(signal_buffer_data(key_buf_p)),
										int(signal_buffer_len(key_buf_p))));
		if (!pstmt_p.exec()) {
			qCritical("Failed to execute statement: \"%s\", error: %d (%s)",
					  pstmt_p.lastQuery().toUtf8().data(),
					  pstmt_p.lastError().number(),
					  pstmt_p.lastError().text().toUtf8().data());
			db().rollback();
			return -3;
		}

		signal_buffer_bzero_free(key_buf_p);
		pstmt_p.finish();

		pre_keys_curr_p = signal_protocol_key_helper_key_list_next(pre_keys_curr_p);
	}

	if (db().commit())
		return 0;
	else
		return -1;
}

}
