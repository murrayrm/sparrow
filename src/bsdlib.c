bcopy(from, to, length)
char *from;
char *to;
int length;
{
    while (length-- > 0) *to++ = *from++;
}

