#!/bin/bash

# формат даты, которую необходимо использовать для сравнения по условию
showtime_time_format_to_compare="date +%s"

# вспомогательная функция для формирования строки вида часы:минуты:секунды по заданному параметру
function ShowtimeTimeFormat() {
  date --date="@$1" "+%-H:%M:%S"
}

# trap debug вызывается дважды: сразу после ввода команды пользователем
# и перед вызовом комманды PROMPT_COMMAND, а меня интересует только первый вызов
# с помощью этой переменной я игнорирую второй вызов
showtime_at_prompt=1

# функция, которая благодаря trap debug вызывается после каждого ввода команды пользователем
# устанавливает значение перемнной showtime_command_start_time в текущее значение времени
function ShowtimePreCommand() {
  if [ -z "$showtime_at_prompt" ]; then
    return
  fi
  unset showtime_at_prompt
  showtime_command_start_time=$(eval "$showtime_time_format_to_compare")
}
trap 'ShowtimePreCommand' DEBUG

# флаг используется для того, чтобы игнорировать первый вызов скрипта в момент
# вызов с помощью source showtime.sh
showtime_first_prompt=1

# функция, которая благодаря PROMPT_COMMAND вызывается после завершения
# введенной пользователем команды
# считывает значение времени записанное функцией ShowtimePreCommand()
# сравнивает с текущим и, если необходимо, формирует строку и выводит с помощью команды echo
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
