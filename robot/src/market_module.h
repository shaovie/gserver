class player;

class market_module
{
public:
  static int dispatch_msg(player *p,
                          const int id,
                          const char *msg,
                          const int len);

  static int handle_get_self_sale_list(player *p, const char *msg, const int len);
  static int handle_error_ret(player *, const char *, const int );

  static void get_market_list(player *p);
  static void sort_by_money(player *p);
  static void sale_market_item(player *p);
  static void sale_market_money(player *p);
  static void get_self_sale_list(player *p);
};
