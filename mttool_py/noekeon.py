"""
 Noekeon Python implementation
 Based on Noekeon specification
"""

NOEKEON_KEY = [0, 0, 0, 0]

# Nr
NUMBER_OF_ROUNDS = 16

WORKING_KEY = NOEKEON_KEY


def Round(Key, State, K1, K2):
    State[0] = State[0] ^ K1
    State = Theta(Key, State)
    State[0] = State[0] ^ K2
    State = Pi1(State)
    State = Gamma(State)
    State = Pi2(State)
    return State


def Gamma(a):
    a[1] = a[1] ^ (((~a[3]) & (~a[2])) & 0xFFFFFFFF)
    a[0] = a[0] ^ (((a[2]) & (a[1])) & 0xFFFFFFFF)

    tmp = a[3]
    a[3] = a[0]
    a[0] = tmp
    a[2] = a[2] ^ a[0] ^ a[1] ^ a[3]

    a[1] = a[1] ^ (((~a[3]) & (~a[2])) & 0xFFFFFFFF)
    a[0] = a[0] ^ (((a[2]) & (a[1])) & 0xFFFFFFFF)
    return a


def Theta(k, a):
    temp = a[0] ^ a[2]
    temp = temp ^ ROTR32(temp, 8) ^ ROTL32(temp, 8)
    a[1] = a[1] ^ temp
    a[3] = a[3] ^ temp

    a[0] = a[0] ^ k[0]
    a[1] = a[1] ^ k[1]
    a[2] = a[2] ^ k[2]
    a[3] = a[3] ^ k[3]

    temp = a[1] ^ a[3]
    temp = temp ^ ROTR32(temp, 8) ^ ROTL32(temp, 8)
    a[0] = a[0] ^ temp
    a[2] = a[2] ^ temp
    return a


def Pi1(a):
    a[1] = ROTL32(a[1], 1)
    a[2] = ROTL32(a[2], 5)
    a[3] = ROTL32(a[3], 2)
    return a


def Pi2(a):
    a[1] = ROTR32(a[1], 1)
    a[2] = ROTR32(a[2], 5)
    a[3] = ROTR32(a[3], 2)
    return a


# Rotate to left a 32 bit number
# Max n is 31
def ROTL32(v, n):
    if (n <= 31):
        return 0xFFFFFFFF & (((v) << (n)) | ((v) >> (32 - (n))))
    else:
        print "ROTL32 -- Rotation Error"
        return v


# Rotate to right a 32 bit number
# Max n is 31
def ROTR32(v, n):
    if (n <= 31):
        return 0xFFFFFFFF & (((v) >> (n)) | ((v) << (32 - (n))))
    else:
        print "ROTR32 -- Rotation Error"
        return v


# Workingkey format is 4 elements of 32 bit size.
# State format is 4 elements of 32 bit size.
# State corresponds to plainText
def NoekeonEncrypt(WorkingKey, State):
    Roundct = [0x80, 0x1B, 0x36, 0x6C,
               0xD8, 0xAB, 0x4D, 0x9A,
               0x2F, 0x5E, 0xBC, 0x63,
               0xC6, 0x97, 0x35, 0x6A, 0xD4]

    for i in range(NUMBER_OF_ROUNDS):
        State = Round(WorkingKey, State, Roundct[i], 0)

    State[0] = State[0] ^ Roundct[NUMBER_OF_ROUNDS]
    State = Theta(WorkingKey, State)
    return State


# Workingkey format is 4 elements of 32 bit size.
# State format is 4 elements of 32 bit size.
# State corresponds to cipherText
def NoekeonDecrypt(WorkingKey, State):
    Roundct = [0x80, 0x1B, 0x36, 0x6C,
               0xD8, 0xAB, 0x4D, 0x9A,
               0x2F, 0x5E, 0xBC, 0x63,
               0xC6, 0x97, 0x35, 0x6A, 0xD4]

    WorkingKey = Theta([0, 0, 0, 0], WorkingKey)

    for i in range(NUMBER_OF_ROUNDS, 0, -1):
        State = Round(WorkingKey, State, 0, Roundct[i])

    State = Theta(WorkingKey, State)
    State[0] = State[0] ^ Roundct[0]
    return State


"""
print "0x55 binary: %s"%(bin(0x55))
print bin(ROTL32(0x55, 1))
print bin(ROTR32(0x55, 1))
text = ""
for num in NoekeonDecrypt([0,0,0,0],NoekeonEncrypt([0,0,0,0], [0,0,0,0])):
    text = text + "%08x"%(num)

print text
"""
