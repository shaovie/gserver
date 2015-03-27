class player;
class socialer_info;

class social_module
{
public:
  static int dispatch_msg(player *p,
                          const int id,
                          const char *msg,
                          const int len);

  static int obtain_social_list(player *p, const char *msg, const int len);

  static int invite_friend(player *p, const char *msg, const int len);

  static int social_other_char_state(player *p, const char *msg, const int len);

  static int add_socialer(player *p, const char *msg, const int len);

  static int remove_socialer(player *p, const char *msg, const int len);

  static int social_other_char_lvl_up(player *p, const char *msg, const int len);

  //
  static void init(player *p);
  static void destroy(player *p);
  static void do_insert_socialer(player *p, const char relation, socialer_info *s);
  static void do_remove_socialer(player *p, const char relation, const int char_id);
  static socialer_info *find_socialer_info(player *p,
                                           const int char_id,
                                           const char relation);
  static void do_add_random_socialer(player *p);
  static void do_remove_random_socialer(player *p);
};
