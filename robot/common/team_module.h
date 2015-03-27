class player;
class team_info;

class team_module
{
public:
  static int dispatch_msg(player *p,
                          const int id,
                          const char *msg,
                          const int len);

  static int invite_be_a_team(player *p, const char *msg, const int len);

  static int all_member_info(player *p, const char *msg, const int len);

  static int add_member(player *p, const char *msg, const int len);

  static int del_member(player *p, const char *msg, const int len);

  static int leave_team(player *p, const char *msg, const int len);

  static int leader_change(player *p, const char *msg, const int len);

  static int member_online(player *p, const char *msg, const int len);
  static int member_offline(player *p, const char *msg, const int len);
  //
  static void init(player *p);
  static void destroy(player *p);
  static void create_team(player *p);
  static void kick_member(player *p);
  static void quit_team(player *p);
  static void invite_member(player *p);

  static bool team_empty(player *p);
};
