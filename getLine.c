#include "main.h"

/**
 * input_buf - to buffers chained commands
 * @info: the parameter struct
 * @buf: an address of buffer
 * @len: an address of len var
 *
 * Return: the bytes read
 */
ssize_t input_buf(info_t *info, char **buf, size_t *len)
{
	ssize_t t = 0;
	size_t len_p = 0;

	if (!*len) /* if nothing left in the buffer, fill it */
	{
		/*bfree((void **)info->cmd_buf);*/
		free(*buf);
		*buf = NULL;
		signal(SIGINT, sigintHandler);
#if USE_GETLINE
		t = getline(buf, &len_p, stdin);
#else
		t = _getline(info, buf, &len_p);
#endif
		if (t > 0)
		{
			if ((*buf)[t - 1] == '\n')
			{
				(*buf)[t - 1] = '\0'; /* remove trailing newline */
				t--;
			}
			info->linecount_flag = 1;
			remove_comments(*buf);
			build_history_list(info, *buf, info->histcount++);
			/* if (_strchr(*buf, ';')) is this a command chain? */
			{
				*len = t;
				info->cmd_buf = buf;
			}
		}
	}
	return (t);
}

/**
 * get_input - to get a line minus the newline
 * @info: the parameter struct
 *
 * Return: the bytes read
 */
ssize_t get_input(info_t *info)
{
	static char *buf; /* the ';' command chain buffer */
	static size_t a, b, len;
	ssize_t r = 0;
	char **buf_p = &(info->arg), *p;

	_putchar(BUF_FLUSH);
	r = input_buf(info, &buf, &len);
	if (r == -1) /* EOF */
	return (-1);
	if (len)        /* we have commands left in the chain buffer */
	{
	b = a; /* init new iterator to current buf position */
	p = buf + a; /* get pointer for return */

	check_chain(info, buf, &b, a, len);
	for (; b < len; b++) /* iterate to semicolon or end */
	{
		if (is_chain(info, buf, &b))
		break;
	}

	a = b + 1; /* increment past nulled ';'' */
	if (a >= len) /* reached end of buffer? */
	{
		a = len = 0; /* reset position and length */
		info->cmd_buf_type = CMD_NORM;
	}

	*buf_p = p; /* pass back pointer to current command position */
	return (_strlen(p)); /* return length of current command */
	}

	*buf_p = buf; /* else not a chain, pass back buffer from _getline() */
	return (r); /* return length of buffer from _getline() */
}

/**
 * read_buf - to read a buffer
 * @info: the parameter struct
 * @buf: a buffer
 * @i: the size
 *
 * Return: r
 */
ssize_t read_buf(info_t *info, char *buf, size_t *i)
{
	ssize_t j = 0;

	if (*i)
		return (0);
	j = read(info->readfd, buf, READ_BUF_SIZE);
	if (j >= 0)
		*i = j;
	return (j);
}

/**
 * _getline - to get the next line of an input from STDIN
 * @info: the parameter struct
 * @ptr: an address of a pointer to buffer, preallocated or NULL
 * @length: the size of preallocated ptr buffer if not NULL
 *
 * Return: s
 */
int _getline(info_t *info, char **ptr, size_t *length)
{
	static char buf[READ_BUF_SIZE];
	static size_t a, len;
	size_t l;
	ssize_t r = 0, s = 0;
	char *p = NULL, *new_p = NULL, *c;

	p = *ptr;
	if (p && length)
		s = *length;
	if (a == len)
		a = len = 0;

	r = read_buf(info, buf, &len);
	if (r == -1 || (r == 0 && len == 0))
		return (-1);

	c = _strchr(buf + a, '\n');
	l = c ? 1 + (unsigned int)(c - buf) : len;
	new_p = _realloc(p, s, s ? s + l : l + 1);
	if (!new_p) /* MALLOC FAILURE! */
		return (p ? free(p), -1 : -1);

	if (s)
		_strncat(new_p, buf + a, l - a);
	else
		_strncpy(new_p, buf + a, l - a + 1);

	s += l - a;
	a = l;
	p = new_p;

	if (length)
		*length = s;
	*ptr = p;
	return (s);
}

/**
 * sigintHandler - the block ctrl-C
 * @sig_num: a signal number
 *
 * Return: the void
 */
void sigintHandler(__attribute__((unused))int sig_num)
{
	_puts("\n");
	_puts("$ ");
	_putchar(BUF_FLUSH);
}
