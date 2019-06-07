#include <QSqlDatabase>
#include <QSqlQuery>
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


/**
 * Convenience function for opening a db "connection" and at the same time preparing a statement.
 *
 * @param db_pp Will be set to the db connection pointer.
 * @param pstmt_pp Will be set to the pointer of the prepared statement
 * @param stmt The SQL statement.
 * @param user_data_p Optional. The user_data as received from the axolotl interface, will be used to set the database name.
 * @return 0 on success, negative on failure
 */
static QSqlQuery db_conn_open(QSqlDatabase db, const QString &stmt, void * AUserData) {
	SignalProtocol * signalProtocol = reinterpret_cast<SignalProtocol*>(AUserData);

	int ret_val = 0;
	char * err_msg = nullptr;

//  sqlite3 * db_p = (void *) 0;
//  sqlite3_stmt * pstmt_p = (void *) 0;

	if (stmt.isEmpty()) {
		ret_val = -1;
		err_msg = "stmt is empty";
		goto cleanup;
	}

//  ret_val = sqlite3_open(axc_context_get_db_fn(axc_ctx_p), &db_p);
	db.setDatabaseName(signalProtocol->dbFileName());
	if (!db.open()) {
		ret_val = -1;
		err_msg = "Failed to open db_p";
		goto cleanup;
	}

	QSqlQuery sql(db);

// sqlite3_prepare_v2(db_p, stmt, -1, &pstmt_p, (void *) 0)
	if (!sql.prepare(stmt)) {
		ret_val = -2;
		err_msg = "Failed to prepare statement";
		goto cleanup;
	}

	return
//	*db_pp = db_p;
//	*pstmt_pp = pstmt_p;

cleanup:
	if (ret_val) {
		db_conn_cleanup(db_p, (void *) 0, err_msg, __func__, axc_ctx_p);
	}

	return ret_val;
}


OmemoStore::OmemoStore()
{

}
