#ifndef KIWI_FE_WRITE_H
#define KIWI_FE_WRITE_H

/*
 * kiwi.
 *
 * postgreSQL protocol interaction library.
*/

typedef struct kiwi_fe_arg kiwi_fe_arg_t;

struct kiwi_fe_arg
{
	char *name;
	int   len;
};

KIWI_API static inline machine_msg_t*
kiwi_fe_write_startup_message(int argc, kiwi_fe_arg_t *argv)
{
	int size = sizeof(uint32_t) + /* len */
	           sizeof(uint32_t) + /* version */
	           sizeof(uint8_t);   /* last '\0' */
	int i = 0;
	for (; i < argc; i++)
		size += argv[i].len;
	machine_msg_t *msg;
	msg = machine_msg_create(size);
	if (kiwi_unlikely(msg == NULL))
		return NULL;
	char *pos;
	pos = machine_msg_get_data(msg);
	/* len */
	kiwi_write32(&pos, size);
	/* version */
	kiwi_write32(&pos, 196608);
	/* arguments */
	for (i = 0; i < argc; i++)
		kiwi_write(&pos, argv[i].name, argv[i].len);
	/* eof */
	kiwi_write8(&pos, 0);
	return msg;
}

KIWI_API static inline machine_msg_t*
kiwi_fe_write_cancel(uint32_t pid, uint32_t key)
{
	int size = sizeof(uint32_t) + /* len */
	           sizeof(uint32_t) + /* special */
	           sizeof(uint32_t) + /* pid */
	           sizeof(uint32_t);  /* key */
	machine_msg_t *msg;
	msg = machine_msg_create(size);
	if (kiwi_unlikely(msg == NULL))
		return NULL;
	char *pos;
	pos = machine_msg_get_data(msg);
	/* len */
	kiwi_write32(&pos, size);
	/* special */
	kiwi_write32(&pos, 80877102);
	/* pid */
	kiwi_write32(&pos, pid);
	/* key */
	kiwi_write32(&pos, key);
	return msg;
}

KIWI_API static inline machine_msg_t*
kiwi_fe_write_ssl_request(void)
{
	int size = sizeof(uint32_t) + /* len */
	           sizeof(uint32_t);  /* special */
	machine_msg_t *msg;
	msg = machine_msg_create(size);
	if (kiwi_unlikely(msg == NULL))
		return NULL;
	char *pos;
	pos = machine_msg_get_data(msg);
	/* len */
	kiwi_write32(&pos, size);
	/* special */
	kiwi_write32(&pos, 80877103);
	return msg;
}

KIWI_API static inline machine_msg_t*
kiwi_fe_write_terminate(void)
{
	int size = sizeof(kiwi_header_t);
	machine_msg_t *msg;
	msg = machine_msg_create(size);
	if (kiwi_unlikely(msg == NULL))
		return NULL;
	char *pos;
	pos = machine_msg_get_data(msg);
	kiwi_write8(&pos, KIWI_FE_TERMINATE);
	kiwi_write32(&pos, sizeof(uint32_t));
	return msg;
}

KIWI_API static inline machine_msg_t*
kiwi_fe_write_password(char *password, int len)
{
	int size = sizeof(kiwi_header_t) + len;
	machine_msg_t *msg;
	msg = machine_msg_create(size);
	if (kiwi_unlikely(msg == NULL))
		return NULL;
	char *pos;
	pos = machine_msg_get_data(msg);
	kiwi_write8(&pos, KIWI_FE_PASSWORD_MESSAGE);
	kiwi_write32(&pos, sizeof(uint32_t) + len);
	kiwi_write(&pos, password, len);
	return msg;
}

KIWI_API static inline machine_msg_t*
kiwi_fe_write_query(char *query, int len)
{
	int size = sizeof(kiwi_header_t) + len;
	machine_msg_t *msg;
	msg = machine_msg_create(size);
	if (kiwi_unlikely(msg == NULL))
		return NULL;
	char *pos;
	pos = machine_msg_get_data(msg);
	kiwi_write8(&pos, KIWI_FE_QUERY);
	kiwi_write32(&pos, sizeof(uint32_t) + len);
	kiwi_write(&pos, query, len);
	return msg;
}

KIWI_API static inline machine_msg_t*
kiwi_fe_write_parse(char *operator_name, int operator_len,
                    char *query, int query_len,
                    uint16_t typec, int *typev)
{
	uint32_t size = sizeof(kiwi_header_t) + operator_len + query_len +
	                sizeof(uint16_t) +
	                typec * sizeof(uint32_t);
	machine_msg_t *msg;
	msg = machine_msg_create(size);
	if (kiwi_unlikely(msg == NULL))
		return NULL;
	char *pos;
	pos = machine_msg_get_data(msg);
	kiwi_write8(&pos, KIWI_FE_PARSE);
	kiwi_write32(&pos, sizeof(uint32_t) + size);
	kiwi_write(&pos, operator_name, operator_len);
	kiwi_write(&pos, query, query_len);
	kiwi_write16(&pos, typec);
	int i = 0;
	for (; i < typec; i++)
		kiwi_write32(&pos, typev[i]);
	return msg;
}

KIWI_API static inline machine_msg_t*
kiwi_fe_write_bind(char *portal_name, int portal_len,
                   char *operator_name, int operator_len,
                   int argc_call_types, int call_types[],
                   int argc_result_types, int result_types[],
                   int argc, int *argv_len, char **argv)
{
	int size = sizeof(kiwi_header_t) + portal_len + operator_len +
	           sizeof(uint16_t) +                     /* argc_call_types */
	           sizeof(uint16_t) * argc_call_types +   /* call_types */
	           sizeof(uint16_t) +                     /* argc_result_types */
	           sizeof(uint16_t) * argc_result_types + /* result_types */
	           sizeof(uint16_t);                      /* argc */
	int i = 0;
	for (; i < argc; i++) {
		size += sizeof(uint32_t);
		if (argv_len[i] == -1)
			continue;
		size += argv_len[i];
	}

	machine_msg_t *msg;
	msg = machine_msg_create(size);
	if (kiwi_unlikely(msg == NULL))
		return NULL;
	char *pos;
	pos = machine_msg_get_data(msg);

	kiwi_write8(&pos, KIWI_FE_BIND);
	kiwi_write32(&pos, sizeof(uint32_t) + size);
	kiwi_write(&pos, portal_name, portal_len);
	kiwi_write(&pos, operator_name, operator_len);
	kiwi_write16(&pos, argc_call_types);
	for (i = 0; i < argc_call_types; i++)
		kiwi_write16(&pos, call_types[i]);
	kiwi_write16(&pos, argc);
	for (i = 0; i < argc; i++) {
		kiwi_write32(&pos, argv_len[i]);
		if (argv_len[i] == -1)
			continue;
		kiwi_write(&pos, argv[i], argv_len[i]);
	}
	kiwi_write16(&pos, argc_result_types);
	for (i = 0; i < argc_result_types; i++)
		kiwi_write16(&pos, result_types[i]);
	return msg;
}

KIWI_API static inline machine_msg_t*
kiwi_fe_write_describe(uint8_t type, char *name, int name_len)
{
	int size = sizeof(kiwi_header_t) + sizeof(type) + name_len;
	machine_msg_t *msg;
	msg = machine_msg_create(size);
	if (kiwi_unlikely(msg == NULL))
		return NULL;
	char *pos;
	pos = machine_msg_get_data(msg);
	kiwi_write8(&pos, KIWI_FE_DESCRIBE);
	kiwi_write32(&pos, sizeof(uint32_t) + size);
	kiwi_write8(&pos, type);
	kiwi_write(&pos, name, name_len);
	return msg;
}

KIWI_API static inline machine_msg_t*
kiwi_fe_write_execute(char *portal, int portal_len, uint32_t limit)
{
	int size = sizeof(kiwi_header_t) + portal_len + sizeof(limit);
	machine_msg_t *msg;
	msg = machine_msg_create(size);
	if (kiwi_unlikely(msg == NULL))
		return NULL;
	char *pos;
	pos = machine_msg_get_data(msg);
	kiwi_write8(&pos, KIWI_FE_EXECUTE);
	kiwi_write32(&pos, sizeof(uint32_t) + size);
	kiwi_write(&pos, portal, portal_len);
	kiwi_write32(&pos, limit);
	return msg;
}

KIWI_API static inline machine_msg_t*
kiwi_fe_write_sync(void)
{
	int size = sizeof(kiwi_header_t);
	machine_msg_t *msg;
	msg = machine_msg_create(size);
	if (kiwi_unlikely(msg == NULL))
		return NULL;
	char *pos;
	pos = machine_msg_get_data(msg);
	kiwi_write8(&pos, KIWI_FE_SYNC);
	kiwi_write32(&pos, sizeof(uint32_t));
	return msg;
}

#endif /* KIWI_FE_WRITE_H */
