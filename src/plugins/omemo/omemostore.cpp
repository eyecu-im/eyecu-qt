#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>
#include <key_helper.h>
#include "signalprotocol.h"
#include "omemostore.h"

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

// OmemoStore instance
OmemoStore * OmemoStore::FInstance = nullptr;

/**
 * Convenience function for opening a db "connection" and at the same time preparing a statement.
 *
 * @param db_pp Will be set to the db connection pointer.
 * @param stmt The SQL statement.
 * @param user_data_p Optional. The user_data as received from the axolotl interface, will be used to set the database name.
 * @return 0 on success, negative on failure
 */
static bool db_conn_open(QSqlDatabase db, const QString &stmt, QSqlQuery &query, void * AUserData) {
	SignalProtocol * signalProtocol = reinterpret_cast<SignalProtocol*>(AUserData);

	if (stmt.isEmpty()) {
		qCritical("stmt is empty");
		return false;
	}

	db.setDatabaseName(signalProtocol->dbFileName());
	if (!db.open()) {
		qCritical("Failed to open db_p");
		return false;
	}

	QSqlQuery sql(db);

	if (sql.prepare(stmt))
		return sql;

	qCritical("Failed to prepare statement");
	return QSqlQuery();
}

// Session store implementation
void OmemoStore::init(const QString &ADatabaseFileName)
{
	FInstance = new OmemoStore();
}

void OmemoStore::uninit()
{
	if (FInstance)
	{
		delete FInstance;
		FInstance = nullptr;
	}
}

int OmemoStore::axc_db_session_load(signal_buffer ** record,
						signal_buffer ** user_record,
						const signal_protocol_address * address,
						void * user_data) {
	Q_UNUSED(user_record);

	const QString stmt("SELECT * FROM " SESSION_STORE_TABLE_NAME
					   " WHERE " SESSION_STORE_NAME_NAME " IS ?1"
					   " AND " SESSION_STORE_DEVICE_ID_NAME " IS ?2;");

	QSqlDatabase db_p;
	QSqlQuery pstmt_p = db_conn_open(db_p, stmt, user_data);
	if (!pstmt_p.isValid()) return -1;

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

int OmemoStore::axc_db_session_get_sub_device_sessions(signal_int_list ** sessions,
													   const char * name, size_t name_len,
													   void * user_data) {
	const QString stmt("SELECT * FROM " SESSION_STORE_TABLE_NAME " WHERE " SESSION_STORE_NAME_NAME " IS ?1;");

	QSqlDatabase db_p;
	QSqlQuery pstmt_p = db_conn_open(db_p, stmt, user_data);
	if (!pstmt_p.isValid()) return -1;

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

int OmemoStore::axc_db_session_store(const signal_protocol_address *address, uint8_t *record, size_t record_len, uint8_t *user_record, size_t user_record_len, void *user_data) {
	Q_UNUSED(user_record);
	Q_UNUSED(user_record_len);

	const QString stmt("INSERT OR REPLACE INTO " SESSION_STORE_TABLE_NAME " VALUES (:name, :name_len, :device_id, :session_record, :record_len);");

	QSqlDatabase db_p;
	QSqlQuery pstmt_p = db_conn_open(db_p, stmt, user_data);
	if (!pstmt_p.isValid()) return -1;

	pstmt_p.bindValue(":name", QString(address->name));
	pstmt_p.bindValue(":name_len", address->name_len);
	pstmt_p.bindValue(3, address->device_id);
	pstmt_p.bindValue(4, QByteArray(reinterpret_cast<char *>(record), int(record_len)));
	pstmt_p.bindValue(5, record_len);

	if (!pstmt_p.exec()) return -3;

	return 0;
}

int OmemoStore::axc_db_session_contains(const signal_protocol_address *address, void *user_data)
{
	const QString stmt(	"SELECT * FROM " SESSION_STORE_TABLE_NAME
						" WHERE " SESSION_STORE_NAME_NAME " IS ?1"
						" AND " SESSION_STORE_DEVICE_ID_NAME " IS ?2;");

	QSqlDatabase db_p;
	QSqlQuery pstmt_p = db_conn_open(db_p, stmt, user_data);
	if (!pstmt_p.isValid()) return -1;

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

int OmemoStore::axc_db_session_delete(const signal_protocol_address *address, void *user_data)
{
	const QString stmt( "DELETE FROM " SESSION_STORE_TABLE_NAME
						" WHERE " SESSION_STORE_NAME_NAME " IS ?1"
						" AND " SESSION_STORE_DEVICE_ID_NAME " IS ?2;");

	QSqlDatabase db_p;
	QSqlQuery pstmt_p = db_conn_open(db_p, stmt, user_data);
	if (!pstmt_p.isValid()) return -1;

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

int OmemoStore::axc_db_session_delete_all(const char *name, size_t name_len, void *user_data)
{
	Q_UNUSED(name_len);

	const QString stmt("DELETE FROM " SESSION_STORE_TABLE_NAME " WHERE " SESSION_STORE_NAME_NAME " IS ?1;");

	QSqlDatabase db_p;
	QSqlQuery pstmt_p = db_conn_open(db_p, stmt, user_data);
	if (!pstmt_p.isValid()) return -1;

	pstmt_p.bindValue(1, QString(name));

	if (pstmt_p.exec())
		return pstmt_p.numRowsAffected();
	else {
		qCritical("Failed to delete sessions");
		return -4;
	}
}

void OmemoStore::axc_db_session_destroy_store_ctx(void *user_data)
{
	Q_UNUSED(user_data);
}


OmemoStore::OmemoStore(const QString &ADatabaseFileName):
	FDb(QSqlDatabase::addDatabase("QPSQL"))
{
	FDb.setDatabaseName(ADatabaseFileName);
}
