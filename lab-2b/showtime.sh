#!/bin/bash

showtime_time_format_to_compare="date +%s"
function ShowtimeTimeFormat() {
  date --date="@$1" "+%-H:%M:%S"
}

showtime_at_prompt=1
function ShowtimePreCommand() {
  if [ -z "$showtime_at_prompt" ]; then
    return
  fi
  unset showtime_at_prompt
  showtime_command_start_time=$(eval "$showtime_time_format_to_compare")
}
trap 'ShowtimePreCommand' DEBUG

showtime_first_prompt=1

function ShowtimePostCommand() {
  showtime_at_prompt=1

  if [ -n "$showtime_first_prompt" ]; then
    unset showtime_first_prompt
    return
  fi

  local command_start_time=$showtime_command_start_time
  local command_end_time=""
  command_end_time=$(eval "$showtime_time_format_to_compare")

  if [ "$command_start_time" -eq "$command_end_time" ]; then
    return
  fi

  local SEC=1
  local MIN=$((60 * SEC))
  local HOUR=$((60 * MIN))
  local DAY=$((24 * HOUR))

  local command_time=$((command_end_time - command_start_time))
  local num_days=$((command_time / DAY))
  local num_hours=$((command_time % DAY / HOUR))
  local num_mins=$((command_time % HOUR / MIN))
  local num_secs=$((command_time % MIN / SEC))

  local time_diff=""

  if [ $num_days -gt 0 ]; then
    time_diff="${time_diff}${num_days} day "
  fi
  if [ $num_hours -gt 0 ]; then
    time_diff="${time_diff}${num_hours} hour "
  fi
  if [ $num_mins -gt 0 ]; then
    time_diff="${time_diff}${num_mins} min "
  fi
  if [ $num_secs -gt 0 ]; then
    time_diff="${time_diff}${num_secs} sec"
  fi

  local output_str=""
  output_str="[$(ShowtimeTimeFormat "$command_start_time") - $(ShowtimeTimeFormat "$command_end_time") ($time_diff)]"
  local output_str_colored="\033[1;35m${output_str}\033[0m"

  echo -e "${output_str_colored}"
}
PROMPT_COMMAND='ShowtimePostCommand'
