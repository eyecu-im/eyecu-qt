#include <Qt>
#include "signalprotocol.h"

extern "C" {
#include "aes.h"
}

SignalProtocol* SignalProtocol::FInstance(nullptr);

SignalProtocol* SignalProtocol::instance()
{
	return FInstance?FInstance:FInstance=new SignalProtocol();
}

int SignalProtocol::randomFunc(uint8_t *AData, size_t ALen, void *AUserData)
{
	Q_UNUSED(AUserData);

	return 0;
}

int SignalProtocol::hmacSha256InitFunc(void **AHmacContext, const uint8_t *AKey, size_t AKeyLen, void *AUserData)
{
	Q_UNUSED(AUserData);

	hmac_sha256_ctx *ctx = new hmac_sha256_ctx;
	hmac_sha256_init(ctx, AKey, AKeyLen);
	*AHmacContext = ctx;
	return 0;
}

int SignalProtocol::hmacSha256UpdateFunc(void *AHmacContext, const uint8_t *AData, size_t ADataLen, void *AUserData)
{
	Q_UNUSED(AUserData);

	hmac_sha256_ctx *ctx = reinterpret_cast<hmac_sha256_ctx *>(AHmacContext);

	hmac_sha256_update(ctx, AData, ADataLen);
	return 0;
}

int SignalProtocol::hmacSha256FinalFunc(void *AHmacContext, signal_buffer **AOutput, void *AUserData)
{
	Q_UNUSED(AUserData);

	unsigned char hmac[32];
	hmac_sha256_ctx *ctx = reinterpret_cast<hmac_sha256_ctx *>(AHmacContext);

	hmac_sha256_final(ctx, hmac, 32);

	signal_buffer *output_buffer = signal_buffer_create(hmac, 32);
	if(!output_buffer)
	   return SG_ERR_NOMEM;

	*AOutput = output_buffer;

	return SG_SUCCESS;
}

void SignalProtocol::hmacSha256CleanupFunc(void *AHmacContext, void *AUserData)
{
	Q_UNUSED(AUserData);

	hmac_sha256_ctx *ctx = reinterpret_cast<hmac_sha256_ctx *>(AHmacContext);
	delete ctx;
}

int SignalProtocol::sha512DigestInitFunc(void **ADigestContext, void *AUserData)
{
	Q_UNUSED(AUserData);

	sha512_ctx *ctx = new sha512_ctx;
	sha512_init(ctx);
	*ADigestContext = ctx;
	return SG_SUCCESS;
}

int SignalProtocol::sha512DigestUpdateFunc(void *ADigestContext, const uint8_t *AData,
										   size_t ADataLen, void *AUserData)
{
	Q_UNUSED(AUserData);

	sha512_ctx *ctx = reinterpret_cast<sha512_ctx *>(ADigestContext);
	sha512_update(ctx, AData, ADataLen);

	return SG_SUCCESS;
}

int SignalProtocol::sha512DigestFinalFunc(void *ADigestContext, signal_buffer **AOutput,
										  void *AUserData)
{
	Q_UNUSED(AUserData);

	unsigned char digest[64];
	sha512_ctx *ctx = reinterpret_cast<sha512_ctx *>(ADigestContext);
	sha512_final(ctx, digest);

	signal_buffer *output_buffer = signal_buffer_create(digest, 64);
	if(!output_buffer)
	   return SG_ERR_NOMEM;

	*AOutput = output_buffer;

	return SG_SUCCESS;
}

void SignalProtocol::sha512DigestCleanupFunc(void *ADigestContext, void *AUserData)
{
	Q_UNUSED(AUserData);

	sha512_ctx *ctx = reinterpret_cast<sha512_ctx*>(ADigestContext);
	delete ctx;
}

int SignalProtocol::encryptFunc(signal_buffer **AOutput, int ACipher, const uint8_t *AKey,
								size_t AKeyLen, const uint8_t *AIv, size_t AIvLen,
								const uint8_t *APlaintext, size_t APlaintextLen, void *AUserData)
{
	Q_UNUSED(AUserData);

}

int SignalProtocol::decryptFunc(signal_buffer **AOutput, int ACipher, const uint8_t *AKey, size_t AKeyLen, const uint8_t *AIv, size_t AIvLen, const uint8_t *ACiphertext, size_t ACiphertextLen, void *AUserData)
{
	Q_UNUSED(AUserData);

}

SignalProtocol::SignalProtocol()
{
	int rc = signal_context_create(&FGlobalContext, this);

	signal_crypto_provider provider;

//	signal_context_set_crypto_provider(FGlobalContext, &provider);
//	signal_context_set_locking_functions(FGlobalContext, lock_function, unlock_function);
}
