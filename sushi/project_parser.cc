
#line 1 "sushi/project_parser.rl"
//
//  project_parser.rl
//

#include <cstdio>
#include <cassert>
#include <cstdlib>
#include <cctype>
#include <cstring>

#include "sushi.h"

#include "project_parser.h"


#line 55 "sushi/project_parser.rl"



#line 23 "sushi/project_parser.cc"
static const char _project_parser_actions[] = {
	0, 1, 0, 1, 1, 1, 2, 1, 
	3, 1, 4, 1, 5, 1, 6, 2, 
	1, 0, 2, 1, 6, 2, 2, 0, 
	2, 2, 6, 2, 5, 0, 2, 5, 
	6
};

static const char _project_parser_key_offsets[] = {
	0, 0, 2, 7, 28, 30, 30, 49, 
	50, 50, 50, 72, 94, 116
};

static const char _project_parser_trans_keys[] = {
	34, 92, 13, 32, 59, 9, 10, 13, 
	32, 34, 39, 59, 60, 92, 95, 123, 
	9, 10, 40, 42, 45, 57, 62, 63, 
	65, 93, 97, 124, 39, 92, 13, 32, 
	59, 60, 92, 95, 124, 9, 10, 40, 
	42, 45, 57, 62, 63, 65, 93, 97, 
	122, 10, 13, 32, 34, 35, 39, 60, 
	92, 95, 124, 125, 9, 10, 40, 42, 
	45, 57, 62, 63, 65, 93, 97, 122, 
	13, 32, 34, 35, 39, 60, 92, 95, 
	124, 125, 9, 10, 40, 42, 45, 57, 
	62, 63, 65, 93, 97, 122, 13, 32, 
	34, 35, 39, 60, 92, 95, 124, 125, 
	9, 10, 40, 42, 45, 57, 62, 63, 
	65, 93, 97, 122, 13, 32, 34, 35, 
	39, 60, 92, 95, 124, 125, 9, 10, 
	40, 42, 45, 57, 62, 63, 65, 93, 
	97, 122, 0
};

static const char _project_parser_single_lengths[] = {
	0, 2, 3, 9, 2, 0, 7, 1, 
	0, 0, 10, 10, 10, 10
};

static const char _project_parser_range_lengths[] = {
	0, 0, 1, 6, 0, 0, 6, 0, 
	0, 0, 6, 6, 6, 6
};

static const char _project_parser_index_offsets[] = {
	0, 0, 3, 8, 24, 27, 28, 42, 
	44, 45, 46, 63, 80, 97
};

static const char _project_parser_indicies[] = {
	1, 2, 0, 3, 3, 5, 3, 4, 
	6, 6, 7, 8, 10, 9, 11, 9, 
	12, 6, 9, 9, 9, 9, 9, 4, 
	1, 14, 13, 13, 15, 15, 17, 16, 
	18, 16, 16, 15, 16, 16, 16, 16, 
	16, 4, 20, 19, 16, 0, 20, 20, 
	7, 19, 8, 9, 11, 9, 9, 21, 
	20, 9, 9, 9, 9, 9, 4, 22, 
	22, 23, 24, 25, 26, 27, 26, 26, 
	28, 22, 26, 26, 26, 26, 26, 4, 
	29, 29, 30, 31, 32, 33, 34, 33, 
	33, 35, 29, 33, 33, 33, 33, 33, 
	4, 36, 36, 37, 38, 39, 40, 41, 
	40, 40, 42, 36, 40, 40, 40, 40, 
	40, 4, 0
};

static const char _project_parser_trans_targs[] = {
	1, 2, 9, 3, 0, 11, 3, 1, 
	4, 6, 11, 8, 13, 4, 5, 3, 
	6, 11, 8, 7, 10, 12, 10, 1, 
	7, 4, 6, 8, 12, 10, 1, 7, 
	4, 6, 8, 12, 10, 1, 7, 4, 
	6, 8, 12
};

static const char _project_parser_trans_actions[] = {
	0, 0, 0, 9, 0, 9, 0, 1, 
	1, 1, 0, 1, 0, 0, 0, 7, 
	0, 7, 0, 0, 0, 0, 11, 27, 
	11, 27, 27, 27, 11, 5, 21, 5, 
	21, 21, 21, 5, 3, 15, 3, 15, 
	15, 15, 3
};

static const char _project_parser_eof_actions[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 13, 30, 24, 18
};

static const int project_parser_start = 10;
static const int project_parser_first_final = 10;
static const int project_parser_error = 0;

static const int project_parser_en_main = 10;


#line 58 "sushi/project_parser.rl"

bool project_parser::parse(const char *buffer, size_t len)
{
	int cs = project_parser_en_main;
	
	const char *mark = NULL;
	const char *p = buffer;
	const char *pe = buffer + strlen(buffer);
	const char *eof = pe;

	
#line 133 "sushi/project_parser.cc"
	{
	cs = project_parser_start;
	}

#line 69 "sushi/project_parser.rl"
	
#line 140 "sushi/project_parser.cc"
	{
	int _klen;
	unsigned int _trans;
	const char *_acts;
	unsigned int _nacts;
	const char *_keys;

	if ( p == pe )
		goto _test_eof;
	if ( cs == 0 )
		goto _out;
_resume:
	_keys = _project_parser_trans_keys + _project_parser_key_offsets[cs];
	_trans = _project_parser_index_offsets[cs];

	_klen = _project_parser_single_lengths[cs];
	if ( _klen > 0 ) {
		const char *_lower = _keys;
		const char *_mid;
		const char *_upper = _keys + _klen - 1;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + ((_upper-_lower) >> 1);
			if ( (*p) < *_mid )
				_upper = _mid - 1;
			else if ( (*p) > *_mid )
				_lower = _mid + 1;
			else {
				_trans += (unsigned int)(_mid - _keys);
				goto _match;
			}
		}
		_keys += _klen;
		_trans += _klen;
	}

	_klen = _project_parser_range_lengths[cs];
	if ( _klen > 0 ) {
		const char *_lower = _keys;
		const char *_mid;
		const char *_upper = _keys + (_klen<<1) - 2;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + (((_upper-_lower) >> 1) & ~1);
			if ( (*p) < _mid[0] )
				_upper = _mid - 2;
			else if ( (*p) > _mid[1] )
				_lower = _mid + 2;
			else {
				_trans += (unsigned int)((_mid - _keys)>>1);
				goto _match;
			}
		}
		_trans += _klen;
	}

_match:
	_trans = _project_parser_indicies[_trans];
	cs = _project_parser_trans_targs[_trans];

	if ( _project_parser_trans_actions[_trans] == 0 )
		goto _again;

	_acts = _project_parser_actions + _project_parser_trans_actions[_trans];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 )
	{
		switch ( *_acts++ )
		{
	case 0:
#line 19 "sushi/project_parser.rl"
	{ mark = p; }
	break;
	case 1:
#line 20 "sushi/project_parser.rl"
	{ begin_block(); }
	break;
	case 2:
#line 21 "sushi/project_parser.rl"
	{ end_block(); }
	break;
	case 3:
#line 22 "sushi/project_parser.rl"
	{ symbol(mark, p - mark); }
	break;
	case 4:
#line 23 "sushi/project_parser.rl"
	{ symbol(mark + 1, p - mark - 2); }
	break;
	case 5:
#line 24 "sushi/project_parser.rl"
	{ end_statement(); }
	break;
#line 238 "sushi/project_parser.cc"
		}
	}

_again:
	if ( cs == 0 )
		goto _out;
	if ( ++p != pe )
		goto _resume;
	_test_eof: {}
	if ( p == eof )
	{
	const char *__acts = _project_parser_actions + _project_parser_eof_actions[cs];
	unsigned int __nacts = (unsigned int) *__acts++;
	while ( __nacts-- > 0 ) {
		switch ( *__acts++ ) {
	case 1:
#line 20 "sushi/project_parser.rl"
	{ begin_block(); }
	break;
	case 2:
#line 21 "sushi/project_parser.rl"
	{ end_block(); }
	break;
	case 5:
#line 24 "sushi/project_parser.rl"
	{ end_statement(); }
	break;
	case 6:
#line 26 "sushi/project_parser.rl"
	{ 
		project_done();
		{p++; goto _out; }
	}
	break;
#line 273 "sushi/project_parser.cc"
		}
	}
	}

	_out: {}
	}

#line 70 "sushi/project_parser.rl"

	return (cs != project_parser_error || cs == project_parser_first_final);
}
