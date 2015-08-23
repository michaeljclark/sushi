
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
	0, 0, 7, 15, 22, 29, 36, 44, 
	49, 50, 55, 63, 71, 79, 87, 95, 
	103, 111, 119, 127
};

static const char _project_parser_trans_keys[] = {
	13, 32, 59, 123, 125, 9, 10, 13, 
	32, 35, 59, 123, 125, 9, 10, 13, 
	32, 59, 123, 125, 9, 10, 9, 10, 
	13, 32, 59, 123, 125, 9, 10, 13, 
	32, 59, 123, 125, 9, 10, 13, 32, 
	35, 59, 123, 125, 13, 32, 59, 9, 
	10, 10, 9, 10, 13, 32, 59, 13, 
	32, 35, 59, 123, 125, 9, 10, 13, 
	32, 35, 59, 123, 125, 9, 10, 13, 
	32, 35, 59, 123, 125, 9, 10, 13, 
	32, 35, 59, 123, 125, 9, 10, 13, 
	32, 35, 59, 123, 125, 9, 10, 9, 
	10, 13, 32, 35, 59, 123, 125, 9, 
	10, 13, 32, 35, 59, 123, 125, 13, 
	32, 35, 59, 123, 125, 9, 10, 9, 
	10, 13, 32, 35, 59, 123, 125, 9, 
	10, 13, 32, 35, 59, 123, 125, 0
};

static const char _project_parser_single_lengths[] = {
	0, 5, 6, 5, 7, 7, 8, 3, 
	1, 5, 6, 6, 6, 6, 6, 8, 
	8, 6, 8, 8
};

static const char _project_parser_range_lengths[] = {
	0, 1, 1, 1, 0, 0, 0, 1, 
	0, 0, 1, 1, 1, 1, 1, 0, 
	0, 1, 0, 0
};

static const unsigned char _project_parser_index_offsets[] = {
	0, 0, 7, 15, 22, 30, 38, 47, 
	52, 54, 60, 68, 76, 84, 92, 100, 
	109, 118, 126, 135
};

static const char _project_parser_indicies[] = {
	1, 1, 2, 3, 3, 1, 0, 5, 
	5, 6, 7, 8, 3, 5, 4, 3, 
	3, 3, 3, 3, 3, 0, 10, 11, 
	10, 10, 10, 10, 10, 9, 12, 13, 
	12, 12, 14, 10, 10, 9, 16, 17, 
	16, 16, 18, 19, 20, 10, 15, 21, 
	21, 22, 21, 3, 11, 10, 23, 24, 
	23, 23, 25, 10, 11, 11, 18, 3, 
	3, 21, 11, 4, 27, 27, 28, 3, 
	3, 29, 27, 26, 17, 17, 18, 7, 
	8, 21, 17, 4, 31, 31, 32, 3, 
	3, 33, 31, 30, 35, 35, 36, 3, 
	3, 37, 35, 34, 39, 27, 39, 39, 
	28, 10, 10, 40, 38, 41, 11, 41, 
	41, 18, 10, 10, 23, 15, 24, 24, 
	18, 22, 3, 21, 24, 4, 43, 35, 
	43, 43, 36, 10, 10, 44, 42, 46, 
	31, 46, 46, 32, 10, 10, 47, 45, 
	0
};

static const char _project_parser_trans_targs[] = {
	1, 2, 11, 0, 1, 2, 3, 11, 
	13, 5, 8, 10, 6, 12, 15, 5, 
	6, 12, 4, 15, 19, 7, 14, 9, 
	17, 18, 1, 10, 4, 7, 1, 10, 
	4, 7, 1, 10, 4, 7, 5, 16, 
	9, 16, 5, 16, 9, 5, 16, 9
};

static const char _project_parser_trans_actions[] = {
	0, 7, 7, 0, 1, 0, 1, 0, 
	0, 0, 0, 0, 7, 7, 7, 1, 
	0, 0, 1, 0, 0, 0, 0, 0, 
	0, 0, 25, 9, 25, 9, 13, 3, 
	13, 3, 19, 5, 19, 5, 25, 9, 
	9, 0, 19, 5, 5, 13, 3, 3
};

static const char _project_parser_eof_actions[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 11, 28, 11, 16, 22, 28, 
	11, 11, 22, 16
};

static const int project_parser_start = 10;
static const int project_parser_first_final = 10;
static const int project_parser_error = 0;

static const int project_parser_en_main = 10;


#line 43 "sushi/project_parser.rl"

bool project_parser::parse(const char *buffer, size_t len)
{
    int cs = project_parser_en_main;
    
    const char *mark = NULL;
    const char *p = buffer;
    const char *pe = buffer + strlen(buffer);
    const char *eof = pe;

    
#line 138 "sushi/project_parser.cc"
	{
	cs = project_parser_start;
	}

#line 54 "sushi/project_parser.rl"
    
#line 145 "sushi/project_parser.cc"
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
	{ start_block(); }
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
#line 239 "sushi/project_parser.cc"
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
	{ start_block(); }
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
#line 274 "sushi/project_parser.cc"
		}
	}
	}

	_out: {}
	}

#line 55 "sushi/project_parser.rl"

    return (cs != project_parser_error && cs == project_parser_first_final);
}
