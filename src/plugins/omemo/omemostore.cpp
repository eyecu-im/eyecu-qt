#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <key_helper.h>
#include "signalprotocol.h"
#include "omemostore.h"

#define INIT_STATUS "init_status"
#define OWN_PUBLIC_KEY "own_public_key"
#define OWN_PRIVATE_KEY "own_private_key"
#define REG_ID "axolotl_registration_id"

#define SESSION_STORE_TABLE "session_store"
#define SESSION_STORE_NAME "name"
#define SESSION_STORE_DEVICE_ID "device_id"
#define SESSION_STORE_RECORD "session_record"
#define PRE_KEY_STORE_TABLE "pre_key_store"
#define PRE_KEY_STORE_ID "id"
#define PRE_KEY_STORE_RECORD "pre_key_record"
#define SIGNED_PRE_KEY_STORE_TABLE "signed_pre_key_store"
#define SIGNED_PRE_KEY_STORE_ID "id"
#define SIGNED_PRE_KEY_STORE_RECORD "signed_pre_key_record"
#define IDENTITY_KEY_STORE_TABLE "identity_key_store"
#define IDENTITY_KEY_STORE_NAME "name"
#define IDENTITY_KEY_STORE_DEVICE_ID "device_id"
#define IDENTITY_KEY_STORE_KEY "key"
#define IDENTITY_KEY_STORE_TRUSTED "trusted"
#define SETTINGS_STORE_TABLE "settings"
#define SETTINGS_STORE_NAME "name"
#define SETTINGS_STORE_PROPERTY "property"

#define DBX(X) db(X->connectionName())
#define DB	DBX(reinterpret_cast<const SignalProtocol *>(AUserData))
#define DBS	DBX(ASignalProtocol)
#define SQL_QUERYX(Q,X) QSqlQuery query(QString(Q), X)
#define SQL_QUERY(Q) SQL_QUERYX(Q, DB)
#define SQL_QUERYS(Q) SQL_QUERYX(Q, DBS)
#define QSTRING(C,S) QString::fromUtf8(QByteArray(C,int(S)))

namespace OmemoStore
{
// Session store implementation
void addDatabase(const QString &ADatabaseFileName, const QString &AConnectionName)
{	
	QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", AConnectionName);
	db.setDatabaseName(ADatabaseFileName);
	if (db.open())
		qDebug("Database opened!");
	else
		qCritical("Database open failed!");
}

void removeDatabase(const QString &AConnectionName)
{
	qDebug() << "OmemoStore::removeDatabase(" << AConnectionName << ")";
	QSqlDatabase::database(AConnectionName).close();
	QSqlDatabase::removeDatabase(AConnectionName);
}

QSqlDatabase db(const QString &AConnectionName)
{
	return QSqlDatabase::database(AConnectionName);
}

int propertySet(const QString &AName, const int AValue, SignalProtocol *ASignalProtocol)
{
	// 1 - name of property
	// 2 - value
	SQL_QUERYS("INSERT OR REPLACE INTO " SETTINGS_STORE_TABLE " VALUES (?1, ?2)");

	query.bindValue(0, AName);
	query.bindValue(1, AValue);
	if (!query.exec()) return -3;

	return 0;
}

int propertyGet(const QString &AName, int &AValue, SignalProtocol *ASignalProtocol)
{
	qDebug() << "OmemoStore::propertyGet(" << AName << ", ...)";
	qDebug() << "connection name:" << ASignalProtocol->connectionName();
	SQL_QUERYS("SELECT * FROM " SETTINGS_STORE_TABLE " WHERE name IS ?1");
	qDebug() << "query:" << query.lastQuery();

	query.bindValue(0, AName);
	qDebug() << "bound:" << query.boundValue(0);

	if (query.exec()) {
		if (query.next()) {
			qCritical("Got result!");
			const int temp = query.value(1).toInt();
			// exactly one result
			if (query.next()) {
				qCritical("Too many results");
				return -3;
			}
			AValue = temp;
			return 0;
		} else {
			qCritical("Result not found");
			return 1;
		}
	} else
		return -3;
}

int sessionLoad(signal_buffer ** ARecord, signal_buffer ** AUserRecord,
				const signal_protocol_address * AAddress, void * AUserData)
{
	Q_UNUSED(AUserRecord);

	SQL_QUERY("SELECT * FROM " SESSION_STORE_TABLE
			  " WHERE " SESSION_STORE_NAME " IS ?1"
			  " AND " SESSION_STORE_DEVICE_ID " IS ?2");

	query.bindValue(0, ADDR_NAME(AAddress));
	query.bindValue(1, AAddress->device_id);
	if (query.exec()) {
		if (query.next()) {
			QByteArray data = query.value(2).toByteArray();
			*ARecord = signal_buffer_create(DATA_SIZE(data));
			if (*ARecord == nullptr) {
			  qCritical("Buffer could not be initialised");
			  return -3;
			}
		} else
			return 0; // session not found
	} else {
		qCritical("Failed executing SQL query");
		return -3;
	}

	return 1;
}

int sessionGetSubDeviceSessions(signal_int_list ** ASessions,
								const char * AName, size_t ANameLen,
								void * AUserData)
{
	Q_UNUSED(ANameLen);

	SQL_QUERY("SELECT * FROM " SESSION_STORE_TABLE
			  " WHERE " SESSION_STORE_NAME " IS ?1");

	signal_int_list * sessionList = nullptr;
	int rc = 0;

	query.bindValue(0, QSTRING(AName, ANameLen));
	if (!query.isValid()) {
		qCritical("Failed to bind name when trying to find sub device sessions");
		rc = -21;
		goto cleanup;
	}

	sessionList = signal_int_list_alloc();

	if (query.exec()) {
		while (query.next())
			signal_int_list_push_back(sessionList, query.value(2).toInt());
	} else {
		qCritical("Error while retrieving result rows");
		rc = -3;
		goto cleanup;
	}

	*ASessions = sessionList;
	rc = int(signal_int_list_size(*ASessions));

cleanup:
	if (rc < 0) {
		if (sessionList) {
		  signal_int_list_free(sessionList);
		}
	}

	return rc;
}

int sessionStore(const signal_protocol_address *AAddress,
				 uint8_t *ARecord, size_t ARecordLen,
				 uint8_t *AUserRecord, size_t AUserRecordLen,
				 void *AUserData)
{
	Q_UNUSED(AUserRecord);
	Q_UNUSED(AUserRecordLen);

	SQL_QUERY("INSERT OR REPLACE INTO " SESSION_STORE_TABLE
			  " VALUES (:name, :device_id, :session_record)");

	query.bindValue(0, ADDR_NAME(AAddress));
	query.bindValue(1, AAddress->device_id);
	query.bindValue(2, BYTE_ARRAY(ARecord, ARecordLen));
	if (!query.exec())
	{
		qCritical("QSL query execution failed: \"%s\"; rc=%d (%s)",
					query.lastQuery().toUtf8().data(),
					query.lastError().number(),
					query.lastError().text().toUtf8().data());
		return -3;
	}

	return 0;
}

int sessionContains(const signal_protocol_address *AAddress, void *AUserData)
{
	SQL_QUERY("SELECT * FROM " SESSION_STORE_TABLE
			  " WHERE " SESSION_STORE_NAME " IS ?1"
			  " AND " SESSION_STORE_DEVICE_ID " IS ?2");

	query.bindValue(0, ADDR_NAME(AAddress));
	query.bindValue(1, AAddress->device_id);

	if (query.exec()) {
		if (query.next())
			return 1;
		else
			return 0;
	} else {
		qCritical("Failed executing SQL query");
		return -3;
	}
}

int sessionDelete(const signal_protocol_address *AAddress, void *AUserData)
{
	SQL_QUERY("DELETE FROM " SESSION_STORE_TABLE
			  " WHERE " SESSION_STORE_NAME " IS ?1"
			  " AND " SESSION_STORE_DEVICE_ID " IS ?2");

	query.bindValue(0, ADDR_NAME(AAddress));
	query.bindValue(1, AAddress->device_id);
	if (query.exec()) {
		if (query.numRowsAffected())
			return 1;
		else
			return 0;
	} else {
		qCritical("Failed to delete session");
		return -4;
	}
}

int sessionDeleteAll(const char *AName, size_t ANameLen, void *AUserData)
{
	Q_UNUSED(ANameLen);

	SQL_QUERY("DELETE FROM " SESSION_STORE_TABLE
			  " WHERE " SESSION_STORE_NAME " IS ?1");

	query.bindValue(0, QSTRING(AName, ANameLen));
	if (query.exec())
		return query.numRowsAffected();
	else {
		qCritical("Failed to delete sessions");
		return -4;
	}
}

void sessionDestroyStoreCtx(void *AUserData)
{
	Q_UNUSED(AUserData);
}

// pre key store impl
int preKeyLoad(signal_buffer ** ARecord, uint32_t APreKeyId, void * AUserData)
{
	SQL_QUERY("SELECT * FROM " PRE_KEY_STORE_TABLE
			  " WHERE " PRE_KEY_STORE_ID " IS ?1");

	query.bindValue(0, APreKeyId);
	if (query.exec()) {
		if (query.next()) {
			QByteArray data = query.value(1).toByteArray();
			*ARecord = signal_buffer_create(DATA_SIZE(data));
			if (!*ARecord) {
				qCritical("Buffer could not be initialised");
				return -3;
			}
		} else {
			qCritical("Pre key ID not found");
			return SG_ERR_INVALID_KEY_ID;
		}
	} else {
		qCritical("Failed executing SQL query");
		return -3;
	}

	return SG_SUCCESS;
}

int preKeyStore(uint32_t APreKeyId, uint8_t * ARecord, size_t ARecordLen,
				void * AUserData)
{
	SQL_QUERY("INSERT OR REPLACE INTO " PRE_KEY_STORE_TABLE
			  " VALUES (?1, ?2)");

	query.bindValue(0, APreKeyId);
	query.bindValue(1, BYTE_ARRAY(ARecord,ARecordLen));
	if (!query.exec())
	{
		qCritical("query execution failed: \"%s\"; error=%d (%s)",
				  query.lastQuery().toUtf8().data(),
				  query.lastError().number(),
				  query.lastError().text().toUtf8().data());
		return -3;
	}

	return 0;
}

int preKeyContains(uint32_t APreKey_Id, void * AUserData)
{
	SQL_QUERY("SELECT * FROM " PRE_KEY_STORE_TABLE
			  " WHERE " PRE_KEY_STORE_ID " IS ?1");

	query.bindValue(0, APreKey_Id);

	if (query.exec()) {
		if (query.next())
			return 1;
		else
			return 0;
	} else {
		qCritical("Failed executing SQL query");
		return -3;
	}
}

int preKeyRemove(uint32_t APreKeyId, void * AUserData)
{
	SQL_QUERY("DELETE FROM " PRE_KEY_STORE_TABLE
			  " WHERE " PRE_KEY_STORE_ID " IS ?1");

	query.bindValue(0, APreKeyId);
	if (query.exec()) {
		if (query.numRowsAffected())
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

void preKeyDestroyCtx(void * AUserData)
{
	Q_UNUSED(AUserData);
//	const char stmt[] = "DELETE FROM pre_key_store; VACUUM;";

//	db_exec_quick(stmt, user_data);
}

// signed pre key store impl
int signedPreKeyLoad(signal_buffer ** ARecord, uint32_t ASignedPreKeyId, void * AUserData)
{
	SQL_QUERY("SELECT * FROM " SIGNED_PRE_KEY_STORE_TABLE
			  " WHERE " SIGNED_PRE_KEY_STORE_ID " IS ?1");

	query.bindValue(0, ASignedPreKeyId);
	if (query.exec()) {
		if (query.next()) {
			QByteArray data = query.value(1).toByteArray();
			*ARecord = signal_buffer_create(DATA_SIZE(data));
			if (!*ARecord) {
				qCritical("Buffer could not be initialised");
				return -3;
			}
		} else
			return SG_ERR_INVALID_KEY_ID; // session not found
	} else {
		qCritical("Failed executing SQL query");
		return -3;
	}

	return SG_SUCCESS;
}

int signedPreKeyStore(uint32_t ASignedPreKeyId, uint8_t * ARecord, size_t ARecordLen, void * AUserData)
{
	SQL_QUERY("INSERT OR REPLACE INTO " SIGNED_PRE_KEY_STORE_TABLE
			  " VALUES (?1, ?2)");

	query.bindValue(0, ASignedPreKeyId);
	query.bindValue(1, BYTE_ARRAY(ARecord, ARecordLen));

	if (!query.exec()) return -3;

	return 0;
}

int signedPreKeyContains(uint32_t ASignedPreKeyId, void * AUserData)
{
	SQL_QUERY("SELECT * FROM " SIGNED_PRE_KEY_STORE_TABLE
			  " WHERE " SIGNED_PRE_KEY_STORE_ID " IS ?1");

	query.bindValue(0, ASignedPreKeyId);
	if (query.exec())	{
		if (query.next()) // result exists
			return 1;
		else // no result
			return 0;
	} else {
		qCritical("Failed executing SQL query");
		return -3;
	}
}

int signedPreKeyRemove(uint32_t ASignedPreKeyId, void * AUserData)
{
	SQL_QUERY("DELETE FROM " SIGNED_PRE_KEY_STORE_TABLE
			  " WHERE " SIGNED_PRE_KEY_STORE_ID " IS ?1");

	query.bindValue(0, ASignedPreKeyId);
	if (query.exec())	{
		if (query.numRowsAffected())
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

void signedPreKeyDestroyCtx(void * AUserData)
{
	Q_UNUSED(AUserData);
//	const char stmt[] = "DELETE FROM signed_pre_key_store; VACUUM;";

//	db_exec_quick(stmt, user_data);
}


// Identity key store implementation
/**
 * saves the public and private key by using the api serialization calls, as this format (and not the higher-level key type) is needed by the getter.
 */
int identitySetKeyPair(const ratchet_identity_key_pair * AKeyPair, SignalProtocol *ASignalProtocol) {
	// 1 - name ("public" or "private")
	// 2 - device id
	// 3 - key blob
	// 4 - trusted (1 for true, 0 for false)

	char * errMsg = nullptr;
	int rc = 0;
	signal_buffer * pubkeyBuf = nullptr;
	signal_buffer * privkeyBuf = nullptr;	

	SQL_QUERYS("INSERT INTO " IDENTITY_KEY_STORE_TABLE " VALUES (?1, ?2, ?3, ?4)");

	// public key
	query.bindValue(0, OWN_PUBLIC_KEY);
	if (ec_public_key_serialize(&pubkeyBuf,
								ratchet_identity_key_pair_get_public(AKeyPair))) {
		errMsg = "Failed to allocate memory to serialize the public key";
		rc = SG_ERR_NOMEM;
		goto cleanup;
	}
	query.bindValue(1, ASignalProtocol->getDeviceId());
	query.bindValue(2, SBUF2BARR(pubkeyBuf));
	query.bindValue(3, IdentityKeyOwn);
	if (!query.exec()) {
		errMsg = "Failed to execute query";
		rc = -3;
		goto cleanup;
	}

	if (query.numRowsAffected() != 1) {
		errMsg = "Failed to insert";
		rc = -3;
		goto cleanup;
	}

	// private key
	query.bindValue(0, OWN_PRIVATE_KEY);

	if (ec_private_key_serialize(&privkeyBuf, ratchet_identity_key_pair_get_private(AKeyPair))) {
		errMsg = "Failed to allocate memory to serialize the private key";
		rc = SG_ERR_NOMEM;
		goto cleanup;
	}

	query.bindValue(2, SBUF2BARR(privkeyBuf));
	if (!query.exec()) {
		errMsg = "Failed to execute query";
		rc = -3;
		goto cleanup;
	}

	if (query.numRowsAffected() != 1) {
		errMsg = "Failed to insert";
		rc = -3;
		goto cleanup;
	}

cleanup:
	if (pubkeyBuf)
		signal_buffer_bzero_free(pubkeyBuf);
	if (privkeyBuf)
		signal_buffer_bzero_free(privkeyBuf);

	if (errMsg)
		qCritical(errMsg);

	return rc;
}

int identityGetKeyPair(signal_buffer ** APublicData, signal_buffer ** APrivateData, void * AUserData)
{
	SQL_QUERY("SELECT * FROM " IDENTITY_KEY_STORE_TABLE
			  " WHERE " IDENTITY_KEY_STORE_NAME " IS ?1"
			  "  AND " IDENTITY_KEY_STORE_DEVICE_ID " IS ?2");

	SignalProtocol *signalProtocol = reinterpret_cast<SignalProtocol *>(AUserData);

	char * errMsg = nullptr;
	int rc = 0;
	signal_buffer * pubkeyBuf = nullptr;
	signal_buffer * privkeyBuf = nullptr;

	// public key
	query.bindValue(1, signalProtocol->getDeviceId());
	query.bindValue(0, OWN_PUBLIC_KEY);	
	if (query.exec()) {
		if (query.next()) {
			QByteArray data = query.value(2).toByteArray();
			pubkeyBuf = signal_buffer_create(DATA_SIZE(data));
			if (!pubkeyBuf) {
				errMsg = "Buffer could not be initialised";
				rc = -3;
				goto cleanup;
			}
		} else {
			// public key not found
			errMsg = "Own public key not found";
			rc = SG_ERR_INVALID_KEY_ID;
			goto cleanup;
		}
	} else {
		errMsg = "Failed executing SQL query";
		rc = -3;
		goto cleanup;
	}

	// private key
	query.bindValue(0, OWN_PRIVATE_KEY);
	if (query.exec()) {
		if (query.next()) {
			QByteArray data = query.value(2).toByteArray();
			privkeyBuf = signal_buffer_create(DATA_SIZE(data));

			if (!privkeyBuf) {
				errMsg = "Buffer could not be initialised";
				rc = -3;
				goto cleanup;
			}
		} else {
			// private key not found
			errMsg = "Own private key not found";
			rc = SG_ERR_INVALID_KEY_ID;
			goto cleanup;
		}
	} else {
		errMsg = "Failed executing SQL query";
		rc = -3;
		goto cleanup;
	}

	*APublicData = pubkeyBuf;
	*APrivateData = privkeyBuf;

cleanup:
	if (rc < 0) {
		if (pubkeyBuf)
			signal_buffer_bzero_free(pubkeyBuf);
		if (privkeyBuf)
			signal_buffer_bzero_free(privkeyBuf);
	}

	if (errMsg)
		qCritical(errMsg);
	return rc;
}

int identitySetLocalRegistrationId(SignalProtocol *ASignalProtocol, const uint32_t ARegistrationId)
{
	return (propertySet(REG_ID, int(ARegistrationId), ASignalProtocol)) ? -1 : 0;
}

int identityGetLocalRegistrationId(void * AUserData, uint32_t * ARegistrationId)
{
	SQL_QUERY("SELECT * FROM " SETTINGS_STORE_TABLE
			  " WHERE " SETTINGS_STORE_NAME " IS ?1");

	query.bindValue(0, REG_ID);
	if (query.exec()) {
		if (query.next()) {
			*ARegistrationId = query.value(1).toUInt();
		} else {
			qCritical("Own registration ID not found");
			return -31;
		}
	} else {
		qCritical("Failed executing SQL query");
		return -32;
	}

	return 0;
}

int identitySave(const signal_protocol_address * AAddress, uint8_t * AKeyData,
				 size_t AKeyLen, void * AUserData)
{
	qDebug() << "identitySave({" << ADDR_NAME(AAddress) << "," << AAddress->device_id << "},"
			 << BYTE_ARRAY(AKeyData, AKeyLen).toHex() << ")";
	// 1 - name ("public" or "private" for own keys, name for contacts)
	// 2 - key blob
	// 3 - trusted (1 for true, 0 for false)

	SQL_QUERY(AKeyData?"INSERT OR REPLACE INTO " IDENTITY_KEY_STORE_TABLE
					   " VALUES (?1, ?2, ?3, ?4)"
					  :"DELETE FROM " IDENTITY_KEY_STORE_TABLE
					   " WHERE " IDENTITY_KEY_STORE_NAME " IS ?1"
					   "  AND " IDENTITY_KEY_STORE_DEVICE_ID " IS ?2");

	query.bindValue(0, ADDR_NAME(AAddress));
	query.bindValue(1, AAddress->device_id);
	if (AKeyData) {
		query.bindValue(2, BYTE_ARRAY(AKeyData, AKeyLen));
		qDebug() << "Saving key as TRUSTED";
		query.bindValue(3, IdentityKeyTrusted);
	}
	if (!query.exec()) return -3;

	return 0;
}

int identityIsTrusted(const signal_protocol_address *AAddress, uint8_t *AKeyData,
					  size_t AKeyLen, void *AUserData)
{
	qDebug() << "identityIsTrusted({" << ADDR_NAME(AAddress) << "," << AAddress->device_id << "},"
			 << BYTE_ARRAY(AKeyData, AKeyLen).toHex() << ")";

	SQL_QUERY("SELECT * FROM " IDENTITY_KEY_STORE_TABLE
			   " WHERE " IDENTITY_KEY_STORE_NAME " IS ?1"
			   "  AND "  IDENTITY_KEY_STORE_DEVICE_ID " IS ?2");

	query.bindValue(0, ADDR_NAME(AAddress));
	query.bindValue(1, AAddress->device_id);
	if (query.exec()) {
		if (query.next()) {
			if (BYTE_ARRAY(AKeyData, AKeyLen) != query.value(2).toByteArray()) {
				qCritical("Key data does not match");
				return 0;
			}
			qDebug() << "Key data matches. Trusted flag:" << query.value(3).toUInt();
			return query.value(3).toUInt()==1;
		} else {
			qDebug() << "No entry. TRUSTED!";
			return 1; // no entry = trusted, according to docs
//			SignalProtocol *signalProtocol = reinterpret_cast<SignalProtocol *>(AUserData);
//			if (signalProtocol->onNewKeyReceived(ADDR_NAME(AAddress),
//												 BYTE_ARRAY(AKeyData, AKeyLen)))
//			{
//				qDebug() << "returning 1";
//				return 1;
//			}
//			else
//			{
//				qDebug() << "returning 0";
//				return 0;
//			}
		}
	} else {
		qCritical("Failed executing SQL query");
		return -32;
	}
}

int identityAlwaysTrusted(const signal_protocol_address * AAddress, uint8_t * AKeyData,
						  size_t AKeyLen, void * AUserData)
{
	Q_UNUSED(AAddress);
	Q_UNUSED(AKeyData);
	Q_UNUSED(AKeyLen);
	Q_UNUSED(AUserData);

	return 1;
}

void identityDestroyCtx(void * AUserData)
{
	Q_UNUSED(AUserData);
//	const char stmt[] = "DELETE FROM identity_key_store; VACUUM;";
//	db_exec_quick(stmt, user_data);
}

int create(SignalProtocol * ASignalProtocol)
{
	const QStringList stmts(
		QStringList() << "CREATE TABLE IF NOT EXISTS " SESSION_STORE_TABLE "("
							 SESSION_STORE_NAME " TEXT NOT NULL, "
							 SESSION_STORE_DEVICE_ID " INTEGER NOT NULL, "
							 SESSION_STORE_RECORD " BLOB NOT NULL, "
						 "  PRIMARY KEY(" SESSION_STORE_NAME ", " SESSION_STORE_DEVICE_ID "))"
					  << "CREATE TABLE IF NOT EXISTS " PRE_KEY_STORE_TABLE "("
							 PRE_KEY_STORE_ID " INTEGER NOT NULL PRIMARY KEY, "
							 PRE_KEY_STORE_RECORD " BLOB NOT NULL)"
					  << "CREATE TABLE IF NOT EXISTS " SIGNED_PRE_KEY_STORE_TABLE "("
							 SIGNED_PRE_KEY_STORE_ID " INTEGER NOT NULL PRIMARY KEY, "
							 SIGNED_PRE_KEY_STORE_RECORD " BLOB NOT NULL)"
					  << "CREATE TABLE IF NOT EXISTS " IDENTITY_KEY_STORE_TABLE "("
							 IDENTITY_KEY_STORE_NAME " TEXT NOT NULL, "
							 IDENTITY_KEY_STORE_DEVICE_ID " INTEGER NOT NULL, "
							 IDENTITY_KEY_STORE_KEY " BLOB NOT NULL, "
							 IDENTITY_KEY_STORE_TRUSTED " INTEGER NOT NULL, "
						 "  PRIMARY KEY(" IDENTITY_KEY_STORE_NAME ", " IDENTITY_KEY_STORE_DEVICE_ID "))"
					  << "CREATE TABLE IF NOT EXISTS " SETTINGS_STORE_TABLE "("
							 SETTINGS_STORE_NAME " TEXT NOT NULL PRIMARY KEY, "
							 SETTINGS_STORE_PROPERTY " INTEGER NOT NULL)");

	if (!DBS.transaction()) {
		qCritical("Failed to start transaction");
		return -3;
	}

	QSqlQuery query(DBS);
	for (QStringList::ConstIterator it = stmts.constBegin();
		 it != stmts.constEnd(); ++it) {
		query.prepare(*it);
		if (!query.exec()) {
			DBS.rollback();
			return -1;
		}
	}

	if (DBS.commit())
		return 0;
	else
		return -2;
}

/**
 * Drops all tables.
 *
 * @param ASignalProtocol pointer to the SignalProtocol object.
 */
int destroy(SignalProtocol * ASignalProtocol) {
	const QStringList stmts(
		QStringList() << "DROP TABLE IF EXISTS " SESSION_STORE_TABLE
					  << "DROP TABLE IF EXISTS " PRE_KEY_STORE_TABLE
					  << "DROP TABLE IF EXISTS " SIGNED_PRE_KEY_STORE_TABLE
					  << "DROP TABLE IF EXISTS " IDENTITY_KEY_STORE_TABLE
					  << "DROP TABLE IF EXISTS " SETTINGS_STORE_TABLE);

	if (!DBS.transaction()) {
		qCritical("Failed to start transaction");
		return -3;
	}

	QSqlQuery query(DBS);
	for (QStringList::ConstIterator it = stmts.cbegin();
		 it != stmts.constEnd(); ++it) {
		query.prepare(*it);
		if (!query.exec()) {
			DBS.rollback();
			return -1;
		}
	}

	if (DBS.commit())
		return 0;
	else
		return -2;
}

int initStatusSet(int AStatus, SignalProtocol *ASignalProtocol) {
	return propertySet(INIT_STATUS, AStatus, ASignalProtocol);
}

int initStatusGet(int &AInitStatus, SignalProtocol *ASignalProtocol)
{
	qDebug() << "OmemoStore::initStatusGet()";
	qDebug() << "connection name:" << ASignalProtocol->connectionName();
	SQL_QUERYS("SELECT * FROM " SETTINGS_STORE_TABLE);
	qDebug() << "query:" << query.lastQuery();

	if (query.exec()) {
		qDebug() << "query size:" << query.size();
		while (query.next()) {
			qDebug() << "Got result!";
			qDebug() << "name:" << query.value(0);
			qDebug() << "value:" << query.value(1);
			// exactly one result
		}
	}

	return propertyGet(INIT_STATUS, AInitStatus, ASignalProtocol);
}

int preKeyStoreList(signal_protocol_key_helper_pre_key_list_node *APreKeysHead,
					SignalProtocol *ASignalProtocol)
{
	if (!DBS.transaction()) {
		qCritical("Failed to start transaction");
		return -3;
	}

	SQL_QUERYS("INSERT OR REPLACE INTO " PRE_KEY_STORE_TABLE " VALUES (?1, ?2)");

	signal_protocol_key_helper_pre_key_list_node * preKeysCurr = APreKeysHead;
	while (preKeysCurr) {
		signal_buffer * keyBuf = nullptr;
		session_pre_key * preKey = signal_protocol_key_helper_key_list_element(preKeysCurr);
		if (session_pre_key_serialize(&keyBuf, preKey)) {
			qCritical("failed to serialize pre key");
			DBS.rollback();
			return -1;
		}

		query.bindValue(0, session_pre_key_get_id(preKey));
		query.bindValue(1, SBUF2BARR(keyBuf));
		if (!query.exec()) {
			qCritical("Failed to execute query: \"%s\", error: %d (%s)",
					  query.lastQuery().toUtf8().data(),
					  query.lastError().number(),
					  query.lastError().text().toUtf8().data());
			DBS.rollback();
			return -3;
		}

		signal_buffer_bzero_free(keyBuf);
		query.finish();

		preKeysCurr = signal_protocol_key_helper_key_list_next(preKeysCurr);
	}

	if (DBS.commit())
		return 0;
	else
		return -1;
}

int preKeyGetList(size_t AAmount, QMap<quint32, QByteArray> &APreKeys,
				  const SignalProtocol *ASignalProtocol)
{
	int rc = -1;
	char * errMsg = nullptr;
	session_pre_key * preKey = nullptr;
	signal_buffer * preKeyPublicSerialized = nullptr;
	signal_buffer * serializedKeypairData = nullptr;

	SQL_QUERYS("SELECT * FROM " PRE_KEY_STORE_TABLE " ORDER BY " PRE_KEY_STORE_ID " ASC LIMIT ?1");

	query.bindValue(0, AAmount);
	if (query.exec())
	{
		while (query.next()) {
			uint32_t id = query.value(0).toUInt();
			QByteArray data = query.value(1).toByteArray();
			serializedKeypairData = signal_buffer_create(DATA_SIZE(data));
			if (!serializedKeypairData) {
				errMsg = "failed to initialize buffer";
				rc = -3;
				goto cleanup;
			}

			rc = session_pre_key_deserialize(&preKey,
											 signal_buffer_data(serializedKeypairData),
											 signal_buffer_len(serializedKeypairData),
											 ASignalProtocol->globalContext());
			if (rc) {
				errMsg = "failed to deserialize keypair";
				goto cleanup;
			}

			ec_key_pair * preKeyPair = session_pre_key_get_key_pair(preKey);
			ec_public_key * preKeyPublic = ec_key_pair_get_public(preKeyPair);

			rc = ec_public_key_serialize(&preKeyPublicSerialized, preKeyPublic);
			if (rc) {
				errMsg = "failed to serialize public key";
				goto cleanup;
			}

			APreKeys.insert(id, SBUF2BARR(preKeyPublicSerialized));
			signal_buffer_free(preKeyPublicSerialized);
		}
	} else {
		errMsg = "sql error when retrieving keys";
		goto cleanup;
	}

	rc = 0;

cleanup:
	if (rc) {
		qCritical("%s: %d", errMsg, rc);
	}

	signal_buffer_free(serializedKeypairData);
	SIGNAL_UNREF(preKey);

	return rc;
}

int preKeyGetMaxId(uint32_t &AMaxId, SignalProtocol *ASignalProtocol)
{
	char * err_msg = nullptr;
	int ret_val = 0;
	uint32_t id = 0;

	SQL_QUERYS("SELECT MAX(" PRE_KEY_STORE_ID ") FROM " PRE_KEY_STORE_TABLE
			   " WHERE " PRE_KEY_STORE_ID " IS NOT ("
			   "   SELECT MAX(" PRE_KEY_STORE_ID ") FROM " PRE_KEY_STORE_TABLE
			   " )");

	if (query.exec()) {
		if (query.next()) {
			id = query.value(0).toUInt();
			if (!id) {
				err_msg = "db not initialized";
				ret_val = -1;
			} else {
				AMaxId = id;
				ret_val = 0;
			}
		} else {
			err_msg = "No data in database";
			ret_val = -2;
		}
	} else {
		err_msg = "SQL query execution failed";
		ret_val = query.lastError().number();
	}

	if (ret_val)
		qCritical("%s: %d", err_msg, ret_val);

	return ret_val;
}

int preKeyGetCount(SignalProtocol * ASignalProtocol)
{
	int ret_val = 0;
	char * err_msg = nullptr;

	SQL_QUERYS("SELECT count(" PRE_KEY_STORE_ID") FROM " PRE_KEY_STORE_TABLE);

	if (query.exec()) {
		if (query.next()) {
			ret_val = query.value(0).toInt();
		} else {
			err_msg = "SQL query returned empty result";
			ret_val = -2;
		}
	} else {
		err_msg = "SQL query execution failed";
		ret_val = -1;
	}

	if (ret_val<0)
		qCritical("%s: %d", err_msg, ret_val);

	return ret_val;
}

int identityKeyGetList(size_t AAmount, QList<IdentityKey> &AIdentityKeys, const SignalProtocol *ASignalProtocol)
{
	int rc = -1;
	char * errMsg = nullptr;
//	session_pre_key * preKey = nullptr;
//	signal_buffer * preKeyPublicSerialized = nullptr;
//	signal_buffer * serializedKeypairData = nullptr;

	SQL_QUERYS(AAmount?"SELECT * FROM " IDENTITY_KEY_STORE_TABLE " LIMIT ?1"
					  :"SELECT * FROM " IDENTITY_KEY_STORE_TABLE);
	if (AAmount)
		query.bindValue(0, AAmount);
	if (query.exec())
	{
		while (query.next()) {
			uint trusted = query.value(3).toUInt();
			if (trusted != IdentityKeyOwn)
			{
				IdentityKey key;
				key.name = query.value(0).toString();
				key.deviceId = query.value(1).toUInt();
				key.keyData = query.value(2).toByteArray();
				key.trusted = trusted;
				AIdentityKeys.append(key);
			}
		}
	} else {
		errMsg = "sql error when retrieving keys";
		goto cleanup;
	}

	rc = 0;

cleanup:
	if (rc) {
		qCritical("%s: %d", errMsg, rc);
	}

	return rc;
}

}
