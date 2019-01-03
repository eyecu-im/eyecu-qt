#include "signalprotocol.h"

SignalProtocol* SignalProtocol::FInstance(nullptr);

SignalProtocol* SignalProtocol::instance()
{
	return FInstance?FInstance:FInstance=new SignalProtocol();
}

int SignalProtocol::randomFunc(uint8_t *AData, size_t ALen, void *AUserData)
{

}

int SignalProtocol::hmacSha256InitFunc(void **AHmacContext, const uint8_t *AKey, size_t AKeyLen, void *AUserData)
{

}

int SignalProtocol::hmacSha256UpdateFunc(void *AHmacContext, const uint8_t *AData, size_t ADataLen, void *AUserData)
{

}

int SignalProtocol::hmacSha256FinalFunc(void *AHmacContext, signal_buffer **AOutput, void *AUserData)
{

}

void SignalProtocol::hmacSha256CleanupFunc(void *AHmacContext, void *AUserData)
{

}

int SignalProtocol::sha512DigestInitFunc(void **ADigestContext, void *AUserData)
{

}

int SignalProtocol::sha512DigestUpdateFunc(void *ADigestContext, const uint8_t *AData, size_t ADataLen, void *AUserData)
{

}

int SignalProtocol::sha512DigestFinalFunc(void *ADigestContext, signal_buffer **AOutput, void *AUserData)
{

}

void SignalProtocol::sha512DigestCleanupFunc(void *ADigestContext, void *AUserData)
{

}

int SignalProtocol::encryptFunc(signal_buffer **AOutput, int ACipher, const uint8_t *AKey, size_t AKeyLen, const uint8_t *AIv, size_t AIvLen, const uint8_t *APlaintext, size_t APlaintextLen, void *AUserData)
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
