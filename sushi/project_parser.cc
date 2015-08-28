
#line 1 "sushi/project_parser.rl"
//
//  project_parser.rl
//

#include <cstdio>
#include <cassert>
#include <cstdlib>
#include <cctype>
#include <cstring>

#include "project_parser.h"


#line 40 "sushi/project_parser.rl"



#line 21 "sushi/project_parser.cc"
static const char _project_parser_actions[] = {
	0, 1, 0, 1, 1, 1, 2, 1, 
	3, 1, 4, 1, 5, 2, 1, 0, 
	2, 1, 5, 2, 2, 0, 2, 2, 
	5, 2, 4, 0, 2, 4, 5
};

static const char _project_parser_key_offsets[] = {
	0, 0, 8, 16, 17, 25, 33, 41
};

static const char _project_parser_trans_keys[] = {
	13, 32, 35, 59, 123, 125, 9, 10, 
	13, 32, 35, 59, 123, 125, 9, 10, 
	10, 13, 32, 35, 59, 123, 125, 9, 
	10, 13, 32, 35, 59, 123, 125, 9, 
	10, 13, 32, 35, 59, 123, 125, 9, 
	10, 13, 32, 35, 59, 123, 125, 9, 
	10, 0
};

static const char _project_parser_single_lengths[] = {
	0, 6, 6, 1, 6, 6, 6, 6
};

static const char _project_parser_range_lengths[] = {
	0, 1, 1, 0, 1, 1, 1, 1
};

static const char _project_parser_index_offsets[] = {
	0, 0, 8, 16, 18, 26, 34, 42
};

static const char _project_parser_indicies[] = {
	1, 1, 2, 3, 2, 2, 1, 0, 
	5, 5, 2, 6, 7, 2, 5, 4, 
	9, 8, 9, 9, 8, 2, 2, 10, 
	9, 4, 12, 12, 13, 2, 2, 14, 
	12, 11, 16, 16, 17, 2, 2, 18, 
	16, 15, 20, 20, 21, 2, 2, 22, 
	20, 19, 0
};

static const char _project_parser_trans_targs[] = {
	1, 2, 0, 5, 1, 2, 5, 7, 
	3, 4, 6, 1, 4, 3, 6, 1, 
	4, 3, 6, 1, 4, 3, 6
};

static const char _project_parser_trans_actions[] = {
	0, 7, 0, 7, 1, 0, 0, 0, 
	0, 0, 0, 25, 9, 9, 9, 19, 
	5, 5, 5, 13, 3, 3, 3
};

static const char _project_parser_eof_actions[] = {
	0, 0, 0, 0, 11, 28, 22, 16
};

static const int project_parser_start = 4;
static const int project_parser_first_final = 4;
static const int project_parser_error = 0;

static const int project_parser_en_main = 4;


#line 43 "sushi/project_parser.rl"

bool project_parser::parse(const char *buffer, size_t len)
{
    int cs = project_parser_en_main;
    
    const char *mark = NULL;
    const char *p = buffer;
    const char *pe = buffer + strlen(buffer);
    const char *eof = pe;

    
#line 100 "sushi/project_parser.cc"
	{
	cs = project_parser_start;
	}

#line 54 "sushi/project_parser.rl"
    
#line 107 "sushi/project_parser.cc"
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
#line 17 "sushi/project_parser.rl"
	{ mark = p; }
	break;
	case 1:
#line 18 "sushi/project_parser.rl"
	{ begin_block(); }
	break;
	case 2:
#line 19 "sushi/project_parser.rl"
	{ end_block(); }
	break;
	case 3:
#line 20 "sushi/project_parser.rl"
	{ symbol(mark, p - mark); }
	break;
	case 4:
#line 21 "sushi/project_parser.rl"
	{ end_statement(); }
	break;
#line 201 "sushi/project_parser.cc"
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
#line 18 "sushi/project_parser.rl"
	{ begin_block(); }
	break;
	case 2:
#line 19 "sushi/project_parser.rl"
	{ end_block(); }
	break;
	case 4:
#line 21 "sushi/project_parser.rl"
	{ end_statement(); }
	break;
	case 5:
#line 23 "sushi/project_parser.rl"
	{ 
        project_done();
        {p++; goto _out; }
    }
	break;
#line 236 "sushi/project_parser.cc"
		}
	}
	}

	_out: {}
	}

#line 55 "sushi/project_parser.rl"

    return (cs != project_parser_error || cs == project_parser_first_final);
}
