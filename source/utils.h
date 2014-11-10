/* Funções usadas por vários mappers */
int maskaddr(unsigned char bank) {
    if (bank >= PRG * 2) {
        unsigned char i = 0xFF;
        while ((bank & i) >= PRG * 2) {
            i /= 2;
        }
        return bank & i;
    } else {
        return bank;
    }
}
