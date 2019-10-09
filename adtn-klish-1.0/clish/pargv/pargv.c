/*
 * pargv.c
 */
#include "private.h"
#include "lub/string.h"
#include "lub/argv.h"
#include "lub/system.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

/*--------------------------------------------------------- */
/*
 * Search for the specified parameter and return its value
 */
static clish_parg_t *find_parg(clish_pargv_t * this, const char *name)
{
	unsigned i;
	clish_parg_t *result = NULL;

	if (!this || !name)
		return NULL;

	/* scan the parameters in this instance */
	for (i = 0; i < this->pargc; i++) {
		clish_parg_t *parg = this->pargv[i];
		const char *pname = clish_param__get_name(parg->param);

		if (0 == strcmp(pname, name)) {
			result = parg;
			break;
		}
	}
	return result;
}

/*--------------------------------------------------------- */
int clish_pargv_insert(clish_pargv_t * this,
	const clish_param_t * param, const char *value)
{
	if (!this || !param)
		return -1;

	clish_parg_t *parg = find_parg(this, clish_param__get_name(param));

	if (parg) {
		/* release the current value */
		lub_string_free(parg->value);
	} else {
		size_t new_size = ((this->pargc + 1) * sizeof(clish_parg_t *));
		clish_parg_t **tmp;

		/* resize the parameter vector */
		tmp = realloc(this->pargv, new_size);
		this->pargv = tmp;
		/* insert reference to the parameter */
		parg = malloc(sizeof(*parg));
		this->pargv[this->pargc++] = parg;
		parg->param = param;
	}
	parg->value = NULL;
	if (value)
		parg->value = lub_string_dup(value);

	return 0;
}

/*--------------------------------------------------------- */
clish_pargv_t *clish_pargv_new(void)
{
	clish_pargv_t *this;

	this = malloc(sizeof(clish_pargv_t));
	this->pargc = 0;
	this->pargv = NULL;

	return this;
}

/*--------------------------------------------------------- */
clish_pargv_t *clish_pargv_clone(const clish_pargv_t *src)
{
	clish_pargv_t *dst;
	unsigned int i;

	if (!src)
		return NULL;

	dst = clish_pargv_new();
	for (i = 0; i < src->pargc; i++) {
		clish_pargv_insert(dst, src->pargv[i]->param, src->pargv[i]->value);
	}

	return dst;
}

/*--------------------------------------------------------- */
static void clish_pargv_fini(clish_pargv_t * this)
{
	unsigned int i;

	/* cleanup time */
	for (i = 0; i < this->pargc; i++) {
		lub_string_free(this->pargv[i]->value);
		this->pargv[i]->value = NULL;
		free(this->pargv[i]);
	}
	free(this->pargv);
}

/*--------------------------------------------------------- */
void clish_pargv_delete(clish_pargv_t * this)
{
	if (!this)
		return;

	clish_pargv_fini(this);
	free(this);
}

/*--------------------------------------------------------- */
unsigned clish_pargv__get_count(clish_pargv_t * this)
{
	if (!this)
		return 0;
	return this->pargc;
}

/*--------------------------------------------------------- */
clish_parg_t *clish_pargv__get_parg(clish_pargv_t * this, unsigned int index)
{
	if (!this)
		return NULL;
	if (index >= this->pargc)
		return NULL;
	return this->pargv[index];
}

/*--------------------------------------------------------- */
const clish_param_t *clish_pargv__get_param(clish_pargv_t * this,
	unsigned index)
{
	clish_parg_t *tmp;

	if (!this)
		return NULL;
	if (index >= this->pargc)
		return NULL;
	tmp = this->pargv[index];
	return tmp->param;
}

/*--------------------------------------------------------- */
const clish_parg_t *clish_pargv_find_arg(clish_pargv_t * this, const char *name)
{
	if (!this)
		return NULL;
	return find_parg(this, name);
}

/*--------------------------------------------------------- */
