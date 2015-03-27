class player;

class chat_module
{
public:
  static int dispatch_msg(player *p,
                          const int id,
                          const char *msg,
                          const int len);
 
  static int world_chat(player *);
  static int team_chat(player *);
  static int private_chat(player *);
  static int scene_chat(player *p);
  static int flaunt_item(player *p);
private:
  static int channel_chat(player *p, const int msg_id, const char *msg, const int len);
  static int recv_world_chat(player *p, const char *msg, const int len);
  static int recv_private_chat(player *p, const char *msg, const int len);
  static int recv_team_chat(player *p, const char *msg, const int len);
  static int recv_scene_chat(player *p, const char *msg, const int len);
  static int recv_flaunt_item(player *p, const char *msg, const int len);
};
