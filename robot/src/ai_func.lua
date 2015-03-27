require("src/msg")

local ai_function = {
  do_heart_beat,
  do_move,
  do_add_item,
  do_sort_package,
  do_meridians_xiulian,
  do_modify_hp_mp,
  do_use_item,
  do_add_pet,
  do_let_pet_out,
  do_enter_scp,
  do_select_career,
  do_add_skill,
  do_use_skill,
  do_relive,
  do_add_money,
  do_get_ol_gift,
  do_get_scp_zhaohui_exp,
  do_add_exp
}

player_ai_struct = {}

ai_struct = {}
ai_struct.__index = ai_struct
function ai_struct:new()
  local ai = {}
  setmetatable(ai, self)
  ai.do_time_ = 0
  return ai
end

function ai_struct:release()
  self = nil
end

all_ai_struct = {}
all_ai_struct.__index = all_ai_struct
function all_ai_struct:new()
  local aas = {}
  setmetatable(aas, self)
  return aas
end

function all_ai_struct:release()
  for k,v in pairs (self) do
    if v ~= nil then
      v:release()
    end
  end
  self = nil
end

ai_function.do_heart_beat = function (player, now, p_aas)
  local ai = p_aas["heart_beat"]
  if ai == nil then
    ai = ai_struct:new()
    p_aas["heart_beat"] = ai
  end

  if now - ai.do_time_ < 10 then
    return 0
  end

  ai.do_time_ = now

  return player:send_request(msg.REQ_CLT_HEART_BEAT, nil)
end

ai_function.do_move = function (player, now, p_aas)
  local ai = p_aas["move"]
  if ai == nil then
    ai = ai_struct:new()
    p_aas["move"] = ai
  end

  local os = lua_out_stream:new_out_stream()
  lua_out_stream:write_int(os, 1)

  return 0
end

ai_function.do_add_item = function (player, now, p_aas)
  return 0
end

ai_function.do_sort_package = function (player, now, p_aas)
  return 0
end

ai_function.do_meridians_xiulian = function (player, now, p_aas)
  return 0
end

ai_function.do_modify_hp_mp = function (player, now, p_aas)
  return 0
end

ai_function.do_use_item = function (player, now, p_aas)
  return 0
end

ai_function.do_add_pet = function (player, now, p_aas)
  return 0
end

ai_function.do_let_pet_out = function (player, now, p_aas)
  return 0
end

ai_function.do_enter_scp = function (player, now, p_aas)
  return 0
end

ai_function.do_select_career = function (player, now, p_aas)
  return 0
end

ai_function.do_add_skill = function (player, now, p_aas)
  return 0
end

ai_function.do_use_skill = function (player, now, p_aas)
  return 0
end

ai_function.do_relive = function (player, now, p_aas)
  return 0
end

ai_function.do_add_money = function (player, now, p_aas)
  return 0
end

ai_function.do_get_ol_gift = function (player, now, p_aas)
  return 0
end

ai_function.do_get_scp_zhaohui_exp = function (player, now, p_aas)
  return 0
end

ai_function.do_add_exp = function (player, now, p_aas)
  return 0
end

function run_function(pl, now)
  local player = tolua.cast(pl, "player")

  local p_ass = player_ai_struct[player.char_id_]
  if p_ass == nil then
    p_aas = all_ai_struct:new()
    player_ai_struct[player.char_id_] = p_aas
  end

  for k,v in pairs(ai_function) do
    local ret = v(player, now, p_aas)
    if ret < 0 then
      return ret;
    end
  end

  return 0;
end

function release_ai_struct(pl)
  local player = tolua.cast(pl, "player")
  if player_ai_struct[player.char_id_] then
    player_ai_struct[player.char_id_]:release()
  end
end
