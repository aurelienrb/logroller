#include <stdio.h>

static const char * g_file = __FILE__;

const char * f()
{
	static const char * f_file = __FILE__;
	return f_file;
}

int main()
{
	static const char * m_file = __FILE__;

	printf("%p %p %p\n", g_file, f(), m_file);
	return 0;
}
