require("src/msg")

local msg_func = { }

local function do_create_char(player)
  local os = lua_out_stream:new_out_stream()
  lua_out_stream:write_string(os, player.name_)
  lua_out_stream:write_char(os, math.random(3))

  return player:send_request(msg.REQ_CREATE_CHAR, os)
end

local function do_start_game(player)
  local os = lua_out_stream:new_out_stream()
  lua_out_stream:write_int(os, player.char_id_)

  return player:send_request(msg.REQ_START_GAME, os)
end

msg_func[msg.RES_HELLO_WORLD] = function (player, message, len, res)
  if res ~= 0 then
    return -1
  end

  local is = in_stream:new(message, len)
  local char_cnt = lua_in_stream:read_int(is)

  if char_cnt == 0 then
    return do_create_char(player)
  end

  player.char_id_ = lua_in_stream:read_int(is)
  return do_start_game(player)
end

msg_func[msg.RES_CREATE_CHAR] = function (player, message, len, res)
  if res~= 0 then
    return -1
  end

  local is = in_stream:new(message, len)
  player.char_id_ = lua_in_stream:read_int(is)

  return do_start_game(player)
end

msg_func[msg.RES_START_GAME] = function (player, message, len, res)
  local is = in_stream:new(message, len)
  local name = lua_in_stream:read_string(is, 32)
  player.career_  = lua_in_stream:read_char(is)
  player.lvl_     = lua_in_stream:read_short(is)
  local totla_exp = lua_in_stream:read_int64_t(is)
  player.exp_     = lua_in_stream:read_int64_t(is)
  player.scene_cid_ = lua_in_stream:read_int(is)
  player.dir_     = lua_in_stream:read_char(is)
  player.x_       = lua_in_stream:read_short(is)
  player.y_       = lua_in_stream:read_short(is)

  if player:lua_schedule_timer() == -1 then
    return -1
  end

  player.last_heart_beat_time_ = time_util.now

  return player:send_request(msg.REQ_ENTER_GAME, nil)
end

msg_func[msg.RES_CLT_HEART_BEAT] = function (player, message, len, res)
  player.last_heart_beat_time_ = time_util.now
  return 0
end

local function create_every_snap_obj(player, is)
  local obj_clsid = 0
  local obj_id = 0
  local so = nil
  while is.length() > 0 do
    local snap_type = 0
    snap_type = lua_in_stream:read_char(is)
    if snap_type == T_CID_ID then
      obj_clsid = lua_in_stream:read_int(is)
      obj_id = lua_in_stream:read_int(is)
      if obj_clsid == obj_id then
        so = player.player_snap_.find(obj_id)
      else
        so = player.monster_snap_.find(obj_id)
      end
    elseif snap_type == T_BASE_INFO then
    elseif snap_type == T_STATUS then
    end
  end

  if so == nil then
    so = snap_obj:new()
    so.obj_id_    = obj_id
    so.obj_clsid_ = obj_clsid

    if obj_clsid == obj_id then
      player.player_snap_.insert(obj_id, so)
    else
      player.monster_snap_.insert(obj_id, so)
    end
  end
end

msg_func[msg.NTF_PUSH_SNAP_INFO] = function (player, msg, len, res)
  local is = in_stream:new(msg, len)
  while is.length() > 0 do
    create_every_snap_obj(player, is)
  end
  return 0
end

function dispatch_msg(player, id, res, message, len)
  print(id)
  player = tolua.cast(player, "player")
  if msg_func[id] == nil then
    return
  end

  return msg_func[id](player, message, len, res)
end

