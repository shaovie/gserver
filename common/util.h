// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2012-07-03 20:19
 */
//========================================================================

#ifndef UTIL_H_
#define UTIL_H_

// Forward declarations

// Lib header
#include <cmath>
#include <cstdio>
#include <cctype>
#include <cstdlib>
#include <cstring>

#include "def.h"
#include "global_macros.h"

/**
 * @class util
 *
 * @brief
 */
class util
{
public:
  static int max(const int a, const int b)
  {
    if (a > b) return a;
    return b;
  }
  static int min(const int a, const int b)
  {
    if (a < b) return a;
    return b;
  }
  //=
  inline static bool double_equal(const double a, const double b)
  { return fabs(a - b) < 0.000001; }
  //=
  inline static bool match_ip(const char *addr_1, const char *addr_2)
  {
    int src_ip_1 = 0, src_ip_2 = 0, src_ip_3 = 0, src_ip_4 = 0;
    ::sscanf(addr_1, "%d.%d.%d.%d",
             &src_ip_1,
             &src_ip_2,
             &src_ip_3,
             &src_ip_4);
    int dst_ip_1 = 0, dst_ip_2 = 0, dst_ip_3 = 0, dst_ip_4 = 0;
    ::sscanf(addr_2, "%d.%d.%d.%d", &dst_ip_1, &dst_ip_2, &dst_ip_3, &dst_ip_4);
    if ((dst_ip_1 == src_ip_1) && (dst_ip_2 == src_ip_2))
      return true;
    return false;
  }
  inline static void to_lower(char *p)
  {
    while (*p != '\0')
    {
      *p = ::tolower(*p);
      ++p;
    }
  }
  inline static const char *istrchr(const char *s, const char v)
  {
    while (*s != '\0')
    {
      if (*s == v) return s;
      ++s;
    }
    return NULL;
  }
  //= http chunked
  inline static int http_chunked_left(char *bf, const int bf_len, const char *s)
  { return ::snprintf(bf, bf_len, "%s2\r\n{ \r\n", s); }
  inline static int http_chunked_right(char *bf)
  {
    static char c_end[] = "3\r\n }\n\r\n0\r\n\r\n";
    ::strcat(bf, c_end);
    return sizeof(c_end) - 1;
  }
  inline static int http_chunked_body_mid(char *bf,
                                          const int bf_len,
                                          const char *s,
                                          const int s_len)
  { return ::snprintf(bf, bf_len, "%x\r\n%s,\r\n", s_len, s); }
  inline static int http_chunked_body_end(char *bf,
                                          const int bf_len,
                                          const char *s,
                                          const int s_len)
  { return ::snprintf(bf, bf_len, "%x\r\n%s\r\n", s_len, s); }

  //= move
  inline static bool is_out_of_distance(const short f_x,
                                        const short f_y,
                                        const short t_x,
                                        const short t_y,
                                        const int d)
  { return ((abs(f_x - t_x) > d) || (abs(f_y - t_y) > d)); }
  inline static  bool is_out_of_distance(const coord_t &from,
                                         const coord_t &to,
                                         const int d)
  { return is_out_of_distance(from.x_, from.y_, to.x_, to.y_, d); }

  inline static bool is_inside_of_redius(const short f_x,
                                         const short f_y,
                                         const short t_x,
                                         const short t_y,
                                         const int r)
  { return ((abs(f_x - t_x) <= r) && (abs(f_y - t_y) <= r)); }
  inline static bool is_inside_of_redius(const coord_t &from,
                                         const coord_t &to,
                                         const int r)
  { return is_inside_of_redius(from.x_, from.y_, to.x_, to.y_, r); }

  inline static bool is_in_view_area(const short f_x,
                                     const short f_y,
                                     const short t_x,
                                     const short t_y,
                                     const int w,
                                     const int h)
  { return ((abs(f_x - t_x) <= w) && (abs(f_y - t_y) <= h)); }

  // string
  inline static bool _verify_name_string(const char *str)
  {
    while (*str != '\0')
    {
      if (::isspace(*str)
          || *str == '\\'
          || *str == '%'
          )
        return false;
      ++str;
    }
    return true;
  }
  inline static bool _verify_db_string(const char *str)
  {
    while (*str != '\0')
    {
      if (*str == '\''
          || *str == '\\'
          || *str == 26 // ctrl + z
         )
        return false;
      ++str;
    }
    return true;
  }
  inline static bool verify_account(const char *str)
  { return util::_verify_db_string(str) && util::_verify_name_string(str); }
  inline static bool verify_name(const char *str)
  { return util::_verify_db_string(str) && util::_verify_name_string(str); }
  inline static bool verify_db(const char *str)
  { return util::_verify_db_string(str); }
  inline static bool is_json(const char *str)
  {
    while (*str != '\0')
    {
      if (!::isalnum(*str)
          && !::isspace(*str)
          && *str != '['
          && *str != ']'
          && *str != '{'
          && *str != '}'
          && *str != '"'
          && *str != ','
          && *str != '-'
          && *str != '_'
          && *str != ':')
        return false;
      ++str;
    }
    return true;
  }
  inline static char *strstrip_all(const char *in_str, char *out_str, const int len)
  {
    if (in_str == 0 || out_str == 0)
      return 0;

    char *s = out_str;
    for (int i = 0; *in_str && i < len; ++in_str)
    {
      if (::isspace((int)*in_str) == 0)
        *s++ = *in_str;
    }
    return s;
  }
  inline static int get_value(const char *src, const char *sub_str, const int sub_str_len)
  {
    char *p = ::strstr((char*)src, sub_str);
    if (p != NULL)
      return ::atoi(p + sub_str_len);
    return 0;
  }
  inline static char *get_http_value(const char *header,
                                     const char *key,
                                     size_t key_len,
                                     char **v_end)
  {
    char *key_p = ::strcasestr(const_cast<char *>(header), key);
    if (key_p == NULL) return NULL;
    key_p = ::strchr(key_p + key_len, ':');
    if (key_p == NULL || *(key_p + 1) == '\r') return NULL;

    ++key_p;
    key_p += ::strspn(key_p, " \t");

    size_t len = ::strcspn(key_p, "\r\n");
    if (len == 0) return NULL;

    if (v_end != NULL)
    {
      *v_end = key_p + len;

      while (*v_end > key_p && isspace(**v_end))
        --(*v_end);
      ++(*v_end);
    }
    return key_p;
  }
  //
  inline static bool is_sex_ok(const char sex)
  { return sex == SEX_MALE || sex == SEX_FEMALE; }

  inline static bool is_career_ok(const char c)
  { return c == CAREER_LI_LIANG || c == CAREER_MIN_JIE || c == CAREER_ZHI_LI; }

  inline static char calc_next_dir(const short f_x,
                                   const short f_y,
                                   const short t_x,
                                   const short t_y)
  {
    if (f_x == t_x)
    {
      if (f_y > t_y)  return DIR_UP;
      if (f_y < t_y)  return DIR_DOWN;
    }else if (f_x > t_x)
    {
      if (f_y == t_y) return DIR_LEFT;
      if (f_y > t_y)  return DIR_LEFT_UP;
      if (f_y < t_y)  return DIR_LEFT_DOWN;
    }else if (f_x < t_x)
    {
      if (f_y == t_y) return DIR_RIGHT;
      if (f_y > t_y)  return DIR_RIGHT_UP;
      if (f_y < t_y)  return DIR_RIGHT_DOWN;
    }
    return DIR_XX;
  }

  inline static char calc_opponent_dir(const char dir)
  { return (dir + 4) % 8; }
  inline static void cake_rate(const int rate, bool &first, int &rate_1, int &rate_2)
  {
    if (first)
    {
      rate_1 = 1;
      rate_2 = rate;
      first  = false;
    }else
    {
      rate_1 = rate_2 + 1;
      rate_2 = rate_1 + rate - 1;
    }
  }
  inline static bool bit_enable(const int64_t value, const char pos)
  {
    if (pos < 0) return false;
    if (pos > 64) return false;
    int64_t bit = 1 << (pos - 1);
    return (value & bit) ? true : false;
  }
  inline static long int calc_move_time(const int dir,
                                        const int speed)
  {
    // 360P/S
    static double step_distance[] =
    {
      0.0,
      32000.0, 57688.0,  // 20*1000(msec)
      48000.0, 57688.0,
      32000.0, 57688.0,
      48000.0, 57688.0
    };
    return (long int)(step_distance[dir] / (360.0 * speed / 100.0));
  }
#if 0
  inline static bool check_speed(int time_diff,
                                 const int dir,
                                 const double speed)
  {
    // 140P/S
    static double step_distance[] =
    {
      0.0,
      20000.0, 44720.0,  // 20*1000(msec)
      40000.0, 44720.0,
      20000.0, 44720.0,
      40000.0, 44720.0
    };
    if (time_diff == 0) time_diff = 1;
    double spd = step_distance[dir] / time_diff;
    if (spd > ((360.0 * speed) / 100.0))
      return false;
    return true;
  }
#endif
  inline static void set_d_bit(int &data, const int bit, const int value)
  {
    const int tmp1 = (int)pow(10, bit-1);
    const int tmp2 = (int)pow(10, bit);
    data -= (data/tmp1 - data/tmp2*10)*tmp1;
    data += value*tmp1;
  }
  inline static int get_d_bit(int data, const int bit)
  {
    const int tmp1 = (int)pow(10, bit-1);
    const int tmp2 = (int)pow(10, bit);
    return (data/tmp1 - data/tmp2*10);
  }
  inline static bool if_int_plus_over(const int val1, const int val2)
  { return MAX_INVALID_INT - val1 < val2; }
  inline static bool if_int_x_over(const int val1, const int val2)
  { return val1 <= 0 || val2 <= 0 || (double)MAX_INVALID_INT / (double)val1 < (double)val2; }
  inline static int php_htoi(char *s)
  {
    int value = 0;
    int c = 0;

    c = ((unsigned char *)s)[0];
    if (isupper(c))
      c = tolower(c);
    value = (c >= '0' && c <= '9' ? c - '0' : c - 'a' + 10) * 16;

    c = ((unsigned char *)s)[1];
    if (isupper(c))
      c = tolower(c);
    value += c >= '0' && c <= '9' ? c - '0' : c - 'a' + 10;

    return value;
  }
  inline static int php_url_decode(char *str, int len)
  {
    char *dest = str;
    char *data = str;

    while (len--)
    {
      if (*data == '+')
        *dest = ' ';
      else if (*data == '%' && len >= 2
               && isxdigit((int) *(data + 1))
               && isxdigit((int) *(data + 2)))
      {
        *dest = (char) php_htoi(data + 1);
        data += 2;
        len -= 2;
      }else
        *dest = *data;

      data++;
      dest++;
    }
    *dest = '\0';
    return dest - str;
  }

};

#endif // UTIL_H_

