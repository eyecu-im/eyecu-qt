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
	return 0;
}

int SignalProtocol::hmacSha256InitFunc(void **AHmacContext, const uint8_t *AKey, size_t AKeyLen, void *AUserData)
{
	hmac_sha256_ctx *ctx = new hmac_sha256_ctx;
	hmac_sha256_init(ctx, AKey, AKeyLen);
	*AHmacContext = ctx;
	return 0;
}

int SignalProtocol::hmacSha256UpdateFunc(void *AHmacContext, const uint8_t *AData, size_t ADataLen, void *AUserData)
{
	hmac_sha256_update(reinterpret_cast<hmac_sha256_ctx*>(AHmacContext),
					   AData, ADataLen);
	return 0;
}

int SignalProtocol::hmacSha256FinalFunc(void *AHmacContext, signal_buffer **AOutput, void *AUserData)
{
	unsigned char *result = new unsigned char[32];
	hmac_sha256_final(reinterpret_cast<hmac_sha256_ctx*>(AHmacContext),
					  result, 32);
	*AOutput = (signal_buffer *)result;
	return 0;
}

void SignalProtocol::hmacSha256CleanupFunc(void *AHmacContext, void *AUserData)
{
	hmac_sha256_ctx *ctx = reinterpret_cast<hmac_sha256_ctx*>(AHmacContext);
	delete ctx;
}

int SignalProtocol::sha512DigestInitFunc(void **ADigestContext, void *AUserData)
{
	sha512_ctx *ctx = new sha512_ctx;
	sha512_init(ctx);
	*ADigestContext = ctx;
	return 0;
}

int SignalProtocol::sha512DigestUpdateFunc(void *ADigestContext, const uint8_t *AData,
										   size_t ADataLen, void *AUserData)
{
	sha512_update(reinterpret_cast<sha512_ctx*>(ADigestContext),
				  AData, ADataLen);
	return 0;
}

int SignalProtocol::sha512DigestFinalFunc(void *ADigestContext, signal_buffer **AOutput,
										  void *AUserData)
{
	unsigned char *result = new unsigned char[64];
	hmac_sha512_final(reinterpret_cast<hmac_sha512_ctx*>(ADigestContext),
					  result, 64);
	*AOutput = (signal_buffer *)result;
	return 0;
}

void SignalProtocol::sha512DigestCleanupFunc(void *ADigestContext, void *AUserData)
{
	sha512_ctx *ctx = reinterpret_cast<sha512_ctx*>(ADigestContext);
	delete ctx;
}

int SignalProtocol::encryptFunc(signal_buffer **AOutput, int ACipher, const uint8_t *AKey,
								size_t AKeyLen, const uint8_t *AIv, size_t AIvLen,
								const uint8_t *APlaintext, size_t APlaintextLen, void *AUserData)
{

}

int SignalProtocol::decryptFunc(signal_buffer **AOutput, int ACipher, const uint8_t *AKey, size_t AKeyLen, const uint8_t *AIv, size_t AIvLen, const uint8_t *ACiphertext, size_t ACiphertextLen, void *AUserData)
{

}

SignalProtocol::SignalProtocol()
{
	int rc = signal_context_create(&FGlobalContext, this);

	signal_crypto_provider provider;

//	signal_context_set_crypto_provider(FGlobalContext, &provider);
//	signal_context_set_locking_functions(FGlobalContext, lock_function, unlock_function);
}
