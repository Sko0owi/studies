import os

from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes
from cryptography.hazmat.primitives import padding

def wyrocznia(ct):
    decryptor = cipher.decryptor()
    decrypted_pt = decryptor.update(ct) + decryptor.finalize()
    try:
        unpadder = padding.PKCS7(128).unpadder()
        decrypted_pt = unpadder.update(decrypted_pt) + unpadder.finalize()
        return True
    except ValueError:
        return False


key = os.urandom(16)

iv = bytes("encryptionIntVec", 'utf-8')

plaintext = bytes.fromhex("554c5452412054414a4e45204841534c4f203a20534b4f5749313131313131")

print("plaintext:", plaintext.decode('utf-8'))

cipher = Cipher(algorithms.AES128(key), modes.CBC(iv))



padder = padding.PKCS7(128).padder()
padded_pt = padder.update(plaintext) + padder.finalize()
print("plaintext with padding: ",padded_pt)

encryptor = cipher.encryptor()
ct = encryptor.update(padded_pt) + encryptor.finalize()



decryptor = cipher.decryptor()
decrypted_pt = decryptor.update(ct) + decryptor.finalize()

unpadder = padding.PKCS7(128).unpadder()
decrypted_pt = unpadder.update(decrypted_pt) + unpadder.finalize()

print("decoding_test ",decrypted_pt.decode('utf-8'))


ct_to_hack = ct.hex()
ct_to_hack = [ ct_to_hack[i:i+2] for i in range(0, len(ct_to_hack), 2) ] # bytes are standalone

print(ct_to_hack)

hacked_plaintext = ""
Decrypt = [0]*16
for i in range(15,0,-1):
    for prep in range(15,i,-1):
        ct_to_hack[prep] = "{:02x}".format((15-i+1) ^ (15-i) ^ int(ct_to_hack[prep],16))
    val_before_hack = ct_to_hack[i]

    for byte in range(0,256):

        ct_to_hack[i] = "{:02x}".format((15-i+1) ^ byte ^ int(val_before_hack,16))
        
        if wyrocznia(bytes.fromhex(''.join(ct_to_hack))):
            print("mamy TO!")     
            print("prevDebug: ", (15-i+1) ^ byte ^ int(val_before_hack,16),int(val_before_hack,16),byte)
            # print("Debug: ", Decrypt[i], int(val_before_hack,16))
            if(byte == 1): continue
            print("mamy TO! SERIO")     
            hacked_plaintext = "{:02x}".format(byte) + hacked_plaintext
            print("current hacked: ",hacked_plaintext, bytes.fromhex(hacked_plaintext))
            break

print(bytes.fromhex(hacked_plaintext))
print(bytes.fromhex(hacked_plaintext).decode('utf-8'))


