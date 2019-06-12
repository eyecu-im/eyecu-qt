#include <QMutex>
#include <QDateTime>
#include <key_helper.h>
#include "signalprotocol.h"
#include "omemostore.h"

extern "C" {
#include <gcrypt.h>
}

SignalProtocol* SignalProtocol::FInstance(nullptr);

SignalProtocol* SignalProtocol::instance()
{
	return FInstance?FInstance:FInstance=new SignalProtocol();
}

void SignalProtocol::init()
{
	gcry_check_version(nullptr);
	gcry_control(GCRYCTL_SUSPEND_SECMEM_WARN);
	gcry_control (GCRYCTL_INIT_SECMEM, 16384, 0);
	gcry_control (GCRYCTL_RESUME_SECMEM_WARN);
	gcry_control(GCRYCTL_USE_SECURE_RNDPOOL);
	gcry_control (GCRYCTL_INITIALIZATION_FINISHED, 0);
}

void SignalProtocol::generateKeys()
{
	unsigned int startId=1000;
	uint64_t timestamp = QDateTime::currentDateTime().toMSecsSinceEpoch();
	signal_protocol_key_helper_generate_identity_key_pair(&FIdentityKeyPair, FGlobalContext);
	signal_protocol_key_helper_generate_registration_id(&FRegistrationId, 0, FGlobalContext);
	signal_protocol_key_helper_generate_pre_keys(&FPreKeysHead, startId, 100, FGlobalContext);
	signal_protocol_key_helper_generate_signed_pre_key(&FSignedPreKey, FIdentityKeyPair, 5, timestamp, FGlobalContext);

	/* Store identity_key_pair somewhere durable and safe. */
	/* Store registration_id somewhere durable and safe. */

	/* Store pre keys in the pre key store. */
	/* Store signed pre key in the signed pre key store. */
}

QString SignalProtocol::dbFileName() const
{
	return FFileName;
}

void SignalProtocol::setDbFileName(const QString &AFileName)
{
	FFileName = AFileName;
}

signal_context *SignalProtocol::globalContext()
{
	return FGlobalContext;
}

static int choose_aes(int cipher, size_t key_len, int * algo_p, int * mode_p) {
  int algo = 0;
  int mode = 0;

  switch(key_len) {
	case 16:
	  algo = GCRY_CIPHER_AES128;
	  break;
	case 24:
	  algo = GCRY_CIPHER_AES192;
	  break;
	case 32:
	  algo = GCRY_CIPHER_AES256;
	  break;
	default:
	  return SG_ERR_UNKNOWN;
  }

  switch (cipher) {
	case SG_CIPHER_AES_CBC_PKCS5:
	  mode = GCRY_CIPHER_MODE_CBC;
	  break;
	case SG_CIPHER_AES_CTR_NOPADDING:
	  mode = GCRY_CIPHER_MODE_CTR;
	  break;
	default:
	  return SG_ERR_UNKNOWN;
  }

  *algo_p = algo;
  *mode_p = mode;

  return 0;
}

int SignalProtocol::randomFunc(uint8_t *AData, size_t ALen, void *AUserData)
{
	Q_UNUSED(AUserData);

//	SignalProtocol * axc_ctx_p = reinterpret_cast<SignalProtocol*>(AUserData);
	gcry_randomize(AData, ALen, GCRY_STRONG_RANDOM);

	return SG_SUCCESS;
}

int SignalProtocol::hmacSha256InitFunc(void **AHmacContext, const uint8_t *AKey, size_t AKeyLen, void *AUserData)
{
	Q_UNUSED(AUserData);

//	SignalProtocol * axc_ctx_p = reinterpret_cast<SignalProtocol *>(AUserData);
	int ret_val = 0;
	char * err_msg = nullptr;

	gcry_mac_hd_t * hmac_hd_p = nullptr;

	hmac_hd_p = reinterpret_cast<gcry_mac_hd_t *>(malloc(sizeof(gcry_mac_hd_t)));
	if (!hmac_hd_p) {
		err_msg = "could not malloc hmac-sha256 ctx";
		ret_val = SG_ERR_NOMEM;
		goto cleanup;
	}

	ret_val = int(gcry_mac_open(hmac_hd_p, GCRY_MAC_HMAC_SHA256, 0, nullptr));
	if (ret_val) {
		err_msg = "could not create hmac-sha256 ctx";
		goto cleanup;
	}

	ret_val = int(gcry_mac_setkey(*hmac_hd_p, AKey, AKeyLen));
	if (ret_val) {
		err_msg = "could not set key for hmac";
		goto cleanup;
	}

	*AHmacContext = hmac_hd_p;

cleanup:
	if (ret_val) {
		if (ret_val > 0) {
			qCritical("%s: %s (%s: %s)\n", __func__, err_msg, gcry_strsource(ret_val), gcry_strerror(ret_val));
			ret_val = SG_ERR_UNKNOWN;
		} else {
			qCritical("%s: %s\n", __func__, err_msg);
		}

		if (hmac_hd_p) {
			gcry_mac_close(*hmac_hd_p);
			free(hmac_hd_p);
		}
	}

	return ret_val;
}

int SignalProtocol::hmacSha256UpdateFunc(void *AHmacContext, const uint8_t *AData, size_t ADataLen, void *AUserData)
{
	Q_UNUSED(AUserData);

	SignalProtocol * axc_ctx_p = reinterpret_cast<SignalProtocol *>(AUserData);
	(void) axc_ctx_p;

	gcry_mac_write(*reinterpret_cast<gcry_mac_hd_t *>(AHmacContext), AData, ADataLen);

	return SG_SUCCESS;
}

int SignalProtocol::hmacSha256FinalFunc(void *AHmacContext, signal_buffer **AOutput, void *AUserData)
{
	Q_UNUSED(AUserData);

//	SignalProtocol * axc_ctx_p = reinterpret_cast<SignalProtocol *>(AUserData);
	int ret_val = 0;
	char * err_msg = nullptr;

	int algo = GCRY_MAC_HMAC_SHA256;
	size_t mac_len = 0;
	uchar * mac_data_p = nullptr;
	signal_buffer * out_buf_p = nullptr;

	mac_len = gcry_mac_get_algo_maclen(algo);

	mac_data_p = reinterpret_cast<uchar *>(malloc(sizeof(uint8_t) * mac_len));
	if (!mac_data_p) {
		ret_val = SG_ERR_NOMEM;
		err_msg = "failed to malloc mac buf";
		goto cleanup;
	}

	ret_val = gcry_mac_read(*reinterpret_cast<gcry_mac_hd_t *>(AHmacContext), mac_data_p, &mac_len);
	if (ret_val) {
		err_msg = "failed to read mac";
		goto cleanup;
	}

	out_buf_p = signal_buffer_create(mac_data_p, mac_len);
	if (!out_buf_p) {
		ret_val = SG_ERR_NOMEM;
		err_msg = "failed to create mac output buf";
		goto cleanup;
	}

	*AOutput = out_buf_p;

	cleanup:
	if (ret_val) {
		if (ret_val > 0) {
			qCritical("%s: %s (%s: %s)\n", __func__, err_msg, gcry_strsource(ret_val), gcry_strerror(ret_val));
			ret_val = SG_ERR_UNKNOWN;
		} else {
			qCritical("%s: %s\n", __func__, err_msg);
		}
	}
	free(mac_data_p);

	return ret_val;
}

void SignalProtocol::hmacSha256CleanupFunc(void *AHmacContext, void *AUserData)
{
	Q_UNUSED(AUserData);

	gcry_mac_hd_t * mac_hd_p = reinterpret_cast<gcry_mac_hd_t *>(AHmacContext);
	gcry_mac_close(*mac_hd_p);
	free(mac_hd_p);
}

int SignalProtocol::sha512DigestInitFunc(void **ADigestContext, void *AUserData)
{
	Q_UNUSED(AUserData);

//	SignalProtocol * axc_ctx_p = (SignalProtocol *) AUserData;
	int ret_val = 0;
	char * err_msg = nullptr;

	gcry_md_hd_t * hash_hd_p = nullptr;
	hash_hd_p = reinterpret_cast<gcry_md_hd_t*>(malloc(sizeof(gcry_mac_hd_t)));
	if (!hash_hd_p) {
	  err_msg = "could not malloc sha512 ctx";
	  ret_val = SG_ERR_NOMEM;
	  goto cleanup;
	}

	ret_val = int(gcry_md_open(hash_hd_p, GCRY_MD_SHA512, 0));
	if (ret_val) {
	  err_msg = "could not create sha512 ctx";
	  goto cleanup;
	}

	*ADigestContext = hash_hd_p;

  cleanup:
	if (ret_val) {
	  if (ret_val > 0) {
		qCritical("%s: %s (%s: %s)\n", __func__, err_msg, gcry_strsource(ret_val), gcry_strerror(ret_val));
		ret_val = SG_ERR_UNKNOWN;
	  } else {
		qCritical("%s: %s\n", __func__, err_msg);
	  }

	  if (hash_hd_p) {
		gcry_md_close(*hash_hd_p);
		free(hash_hd_p);
	  }
	}

	return ret_val;
}

int SignalProtocol::sha512DigestUpdateFunc(void *ADigestContext, const uint8_t *AData,
										   size_t ADataLen, void *AUserData)
{
	Q_UNUSED(AUserData);

	gcry_md_write(*reinterpret_cast<gcry_md_hd_t *>(ADigestContext), AData, ADataLen);

	return SG_SUCCESS;
}

int SignalProtocol::sha512DigestFinalFunc(void *ADigestContext, signal_buffer **AOutput,
										  void *AUserData)
{
	Q_UNUSED(AUserData);

//	SignalProtocol *signalProtocol = (SignalProtocol *)AUserData;
	gcry_md_hd_t * hash_hd_p = (gcry_md_hd_t *) ADigestContext;
	int ret_val = 0;
	char * err_msg = nullptr;

	int algo = GCRY_MD_SHA512;
	size_t hash_len = 0;
	unsigned char * hash_data_p = nullptr;
	signal_buffer * out_buf_p = nullptr;

	hash_len = gcry_md_get_algo_dlen(algo);

	hash_data_p = gcry_md_read(*hash_hd_p, algo);
	if (!hash_data_p) {
	  ret_val = SG_ERR_UNKNOWN;
	  err_msg = "failed to read hash";
	  goto cleanup;
	}

	out_buf_p = signal_buffer_create((uint8_t *) hash_data_p, hash_len);
	if (!out_buf_p) {
	  ret_val = SG_ERR_NOMEM;
	  err_msg = "failed to create hash output buf";
	  goto cleanup;
	}

	gcry_md_reset(*hash_hd_p);

	*AOutput = out_buf_p;

  cleanup:
  if (ret_val) {
	if (ret_val > 0) {
	  qCritical("%s: %s (%s: %s)\n", __func__, err_msg, gcry_strsource(ret_val), gcry_strerror(ret_val));
	  ret_val = SG_ERR_UNKNOWN;
	} else {
	  qCritical("%s: %s\n", __func__, err_msg);
	}
  }

	return ret_val;
}

void SignalProtocol::sha512DigestCleanupFunc(void *ADigestContext, void *AUserData)
{
	Q_UNUSED(AUserData);

	gcry_md_hd_t * hash_hd_p = reinterpret_cast<gcry_md_hd_t *>(ADigestContext);

	gcry_md_close(*hash_hd_p);
	free(hash_hd_p);
}

int SignalProtocol::encryptFunc(signal_buffer **AOutput, int ACipher, const uint8_t *AKey,
								size_t AKeyLen, const uint8_t *AIv, size_t AIvLen,
								const uint8_t *APlaintext, size_t APlaintextLen, void *AUserData)
{
	Q_UNUSED(AUserData);

	int rc = SG_SUCCESS;
	char * err_msg = nullptr;
//	SignalProtocol * axc_ctx_p = (SignalProtocol *)AUserData;

	int algo = 0;
	int mode = 0;
	size_t pad_len = 0;
	size_t ct_len = 0;
	gcry_cipher_hd_t cipher_hd = {nullptr};
	uchar * pt_p = nullptr;
	uchar * out_p = nullptr;
	signal_buffer * out_buf_p = nullptr;

	if(AIvLen != 16) {
	  err_msg = "invalid AES IV size (must be 16)";
	  rc = SG_ERR_UNKNOWN;
	  goto cleanup;
	}

	rc = choose_aes(ACipher, AKeyLen, &algo, &mode);
	if (rc) {
	  err_msg = "failed to choose cipher";
	  rc = SG_ERR_UNKNOWN;
	  goto cleanup;
	}

	rc = gcry_cipher_open(&cipher_hd, algo, mode, 0);
	if (rc) {
	  err_msg = "failed to init cipher";
	  goto cleanup;
	}

	rc = gcry_cipher_setkey(cipher_hd, AKey, AKeyLen);
	if (rc) {
	  err_msg = "failed to set key";
	  goto cleanup;
	}

	switch (ACipher) {
	  case SG_CIPHER_AES_CBC_PKCS5:
		pad_len = 16 - (APlaintextLen % 16);
		if (pad_len == 0) {
		  pad_len = 16;
		}
		ct_len = APlaintextLen + pad_len;
		rc = int(gcry_cipher_setiv(cipher_hd, AIv, AIvLen));
		if (rc) {
		  err_msg = "failed to set iv";
		  goto cleanup;
		}
		break;
	  case SG_CIPHER_AES_CTR_NOPADDING:
		ct_len = APlaintextLen;
		rc = int(gcry_cipher_setctr(cipher_hd, AIv, AIvLen));
		if (rc) {
		  err_msg = "failed to set iv";
		  goto cleanup;
		}
		break;
	  default:
		rc = SG_ERR_UNKNOWN;
		err_msg = "unknown cipher";
		goto cleanup;
	}

	pt_p = (uint8_t*)malloc(sizeof(uint8_t) * ct_len);
	if (!pt_p) {
	  err_msg = "failed to malloc pt buf";
	  rc = SG_ERR_NOMEM;
	  goto cleanup;
	}
	memset(pt_p, pad_len, ct_len);
	memcpy(pt_p, APlaintext, APlaintextLen);

	out_p = (uchar*)malloc(sizeof(uint8_t) * ct_len);
	if (!out_p) {
	  err_msg = "failed to malloc ct buf";
	  rc = SG_ERR_NOMEM;
	  goto cleanup;
	}

	rc = gcry_cipher_encrypt(cipher_hd, out_p, ct_len, pt_p, ct_len);
	if (rc) {
	  err_msg = "failed to encrypt";
	  goto cleanup;
	}

	out_buf_p = signal_buffer_create(out_p, ct_len);
	*AOutput = out_buf_p;

cleanup:
	if (rc) {
	  if (rc > 0) {
		qCritical("%s: %s (%s: %s)\n", __func__, err_msg, gcry_strsource(rc), gcry_strerror(rc));
		rc = SG_ERR_UNKNOWN;
	  } else {
		qCritical("%s: %s\n", __func__, err_msg);
	  }
	}

	free(out_p);
	gcry_cipher_close(cipher_hd);

	return rc;
}

int SignalProtocol::decryptFunc(signal_buffer **AOutput, int ACipher, const uint8_t *AKey, size_t AKeyLen, const uint8_t *AIv, size_t AIvLen, const uint8_t *ACiphertext, size_t ACiphertextLen, void *AUserData)
{
	Q_UNUSED(AUserData);

	int ret_val = SG_SUCCESS;
	char * err_msg = nullptr;
//	SignalProtocol *axc_ctx_p = reinterpret_cast<SignalProtocol *>(AUserData);

	int algo = 0;
	int mode = 0;
	gcry_cipher_hd_t cipher_hd = {nullptr};
	uchar * out_p = nullptr;
	size_t pad_len = 0;
	signal_buffer * out_buf_p = nullptr;

	if(AIvLen != 16) {
	  err_msg = "invalid AES IV size (must be 16)";
	  ret_val = SG_ERR_UNKNOWN;
	  goto cleanup;
	}

	ret_val = choose_aes(ACipher, AKeyLen, &algo, &mode);
	if (ret_val) {
	  err_msg = "failed to choose cipher";
	  ret_val = SG_ERR_UNKNOWN;
	  goto cleanup;
	}

	ret_val = gcry_cipher_open(&cipher_hd, algo, mode, 0);
	if (ret_val) {
	  err_msg = "failed to init cipher";
	  goto cleanup;
	}

	ret_val = gcry_cipher_setkey(cipher_hd, AKey, AKeyLen);
	if (ret_val) {
	  err_msg = "failed to set key";
	  goto cleanup;
	}

	switch (ACipher) {
	  case SG_CIPHER_AES_CBC_PKCS5:
		pad_len = 1;
		ret_val = gcry_cipher_setiv(cipher_hd, AIv, AIvLen);
		if (ret_val) {
		  err_msg = "failed to set iv";
		  goto cleanup;
		}
		break;
	  case SG_CIPHER_AES_CTR_NOPADDING:
		ret_val = gcry_cipher_setctr(cipher_hd, AIv, AIvLen);
		if (ret_val) {
		  err_msg = "failed to set iv";
		  goto cleanup;
		}
		break;
	  default:
		ret_val = SG_ERR_UNKNOWN;
		err_msg = "unknown cipher";
		goto cleanup;
	}

	out_p = (uchar*)malloc(sizeof(uint8_t) * ACiphertextLen);
	if (!out_p) {
	  err_msg = "failed to malloc pt buf";
	  ret_val = SG_ERR_NOMEM;
	  goto cleanup;
	}

	ret_val = gcry_cipher_decrypt(cipher_hd, out_p, ACiphertextLen, ACiphertext, ACiphertextLen);
	if (ret_val) {
	  err_msg = "failed to decrypt";
	  goto cleanup;
	}

	if (pad_len) {
	  pad_len = out_p[ACiphertextLen - 1];
	}

	out_buf_p = signal_buffer_create(out_p, ACiphertextLen - pad_len);
	*AOutput = out_buf_p;

  cleanup:
	if (ret_val) {
	  if (ret_val > 0) {
		qCritical("%s: %s (%s: %s)\n", __func__, err_msg, gcry_strsource(ret_val), gcry_strerror(ret_val));
		ret_val = SG_ERR_UNKNOWN;
	  } else {
		qCritical("%s: %s\n", __func__, err_msg);
	  }
	}

	free(out_p);
	gcry_cipher_close(cipher_hd);

	return ret_val;
}

void SignalProtocol::recursiveMutexLock(void *AUserData)
{
	reinterpret_cast<SignalProtocol *>(AUserData)->recursiveMutexLock();
}

void SignalProtocol::recursiveMutexUnlock(void *AUserData)
{
	reinterpret_cast<SignalProtocol *>(AUserData)->recursiveMutexUnlock();
}

void SignalProtocol::recursiveMutexLock()
{
	FMutex->lock();
}

void SignalProtocol::recursiveMutexUnlock()
{
	FMutex->unlock();
}

SignalProtocol::SignalProtocol():
	FGlobalContext(nullptr),
	FFileName("omemo.sqlite"),
	FMutex(new QMutex(QMutex::Recursive))
{
	char *err_msg = nullptr;
	signal_protocol_store_context * store_context_p = nullptr;

	// 1. create global context
	if (signal_context_create(&FGlobalContext, this)) {
	  err_msg = "failed to create global signal protocol context";
	  FError = -1;
	  goto cleanup;
	}
	qDebug("%s: created and set signal protocol context", __func__);

	// 2. init and set crypto provider
	signal_crypto_provider provider;
	provider.random_func = randomFunc;
	provider.hmac_sha256_init_func = hmacSha256InitFunc;
	provider.hmac_sha256_update_func = hmacSha256UpdateFunc;
	provider.hmac_sha256_final_func = hmacSha256FinalFunc;
	provider.hmac_sha256_cleanup_func = hmacSha256CleanupFunc;
	provider.sha512_digest_init_func = sha512DigestInitFunc;
	provider.sha512_digest_update_func = sha512DigestUpdateFunc;
	provider.sha512_digest_final_func = sha512DigestFinalFunc;
	provider.sha512_digest_cleanup_func = sha512DigestCleanupFunc;
	provider.encrypt_func = encryptFunc;
	provider.decrypt_func = decryptFunc;
	provider.user_data = this;

	if (signal_context_set_crypto_provider(FGlobalContext, &provider)) {
	  err_msg = "failed to set signal protocol crypto provider";
	  FError = -1;
	  goto cleanup;
	}
	qDebug("%s: set signal protocol crypto provider", __func__);

	// 3. set locking functions
	if (signal_context_set_locking_functions(FGlobalContext, recursiveMutexLock, recursiveMutexUnlock)) {
	  err_msg = "failed to set locking functions";
	  FError = -1;
	  goto cleanup;
	}
	qDebug("%s: set locking functions", __func__);

	// init store context

	if (signal_protocol_store_context_create(&store_context_p, FGlobalContext)) {
	  err_msg = "failed to create store context";
	  FError = -1;
	  goto cleanup;
	}

	qDebug("%s: created store context", __func__);

	signal_protocol_session_store session_store;
	session_store.load_session_func = &OmemoStore::axc_db_session_load;
	session_store.get_sub_device_sessions_func = &OmemoStore::axc_db_session_get_sub_device_sessions;
	session_store.store_session_func = &OmemoStore::axc_db_session_store;
	session_store.contains_session_func = &OmemoStore::axc_db_session_contains;
	session_store.delete_session_func = &OmemoStore::axc_db_session_delete;
	session_store.delete_all_sessions_func = &OmemoStore::axc_db_session_delete_all;
	session_store.destroy_func = &OmemoStore::axc_db_session_destroy_store_ctx;
	session_store.user_data = this;

	if (signal_protocol_store_context_set_session_store(store_context_p, &session_store)) {
	  err_msg = "failed to create session store";
	  FError = -1;
	  goto cleanup;
	}

	signal_protocol_pre_key_store pre_key_store;
	pre_key_store.load_pre_key = &OmemoStore::axc_db_pre_key_load;
	pre_key_store.store_pre_key = &OmemoStore::axc_db_pre_key_store;
	pre_key_store.contains_pre_key = &OmemoStore::axc_db_pre_key_contains;
	pre_key_store.remove_pre_key = &OmemoStore::axc_db_pre_key_remove;
	pre_key_store.destroy_func = &OmemoStore::axc_db_pre_key_destroy_ctx;
	pre_key_store.user_data = this;

	if (signal_protocol_store_context_set_pre_key_store(store_context_p, &pre_key_store)) {
	  err_msg = "failed to set pre key store";
	  FError = -1;
	  goto cleanup;
	}

	signal_protocol_signed_pre_key_store signed_pre_key_store;
	signed_pre_key_store.load_signed_pre_key = &OmemoStore::axc_db_signed_pre_key_load;
	signed_pre_key_store.store_signed_pre_key = &OmemoStore::axc_db_signed_pre_key_store;
	signed_pre_key_store.contains_signed_pre_key = &OmemoStore::axc_db_signed_pre_key_contains;
	signed_pre_key_store.remove_signed_pre_key = &OmemoStore::axc_db_signed_pre_key_remove;
	signed_pre_key_store.destroy_func = &OmemoStore::axc_db_signed_pre_key_destroy_ctx;
	signed_pre_key_store.user_data = this;

	if (signal_protocol_store_context_set_signed_pre_key_store(store_context_p, &signed_pre_key_store)) {
	  err_msg = "failed to set signed pre key store";
	  FError = -1;
	  goto cleanup;
	}

//	if (signal_protocol_store_context_set_identity_key_store(store_context_p, &identity_key_store)) {
//	  err_msg = "failed to set identity key store";
//	  FError = -1;
//	  goto cleanup;
//	}

//	ctx_p->axolotl_store_context_p = store_context_p;
//	axc_log(ctx_p, AXC_LOG_DEBUG, "%s: set store context", __func__);

  cleanup:
	if (FError < 0) {
		//FIXME: this frees inited context, make this more fine-grained
//		axc_cleanup(ctx_p);
	  qCritical("%s: %s", __func__, err_msg);
	} else {
		OmemoStore::init("OMEMO");
		qInfo("%s: done initializing axc", __func__);
	}
}

SignalProtocol::~SignalProtocol()
{
	OmemoStore::uninit();
}

axc_buf_list_item::axc_buf_list_item(uint32_t AId, signal_buffer *ABuf_p):
	id(AId), buf_p(ABuf_p)
{}
