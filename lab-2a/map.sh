#!/bin/sh

# set -xe -o pipefail

usage ()
{
  echo "Usage : map.sh [OPTION]... FILE"
  echo "Run several copies of command in parallel passing lines from file as arguments"
  echo ""
  echo "Mandatory arguments to long options are mandatory for short options too."
  echo "    -c, --cmd c	       command, that should be executed"
  echo "		       (default command is 'echo')"
  echo "    -i, --input	       enable mode when command recieves arguments through standard input"
  echo "    -t, --threads t    number of command's copies, that can be executed in parallel"
  echo "		       (should be greater or equal to 1, default value is 1)"
  exit
}

# Переменные для входных данных
# Файл, из которого будем производиться чтение
file=
# Команда, которая будет выполняться для строк из файлы
command=echo
# Передача строки как аргумента или на стандартный вход
standart_input=0
# Количество копий команды, запускаемых одновременно
num_threads_in_parallel=1

# Парсинг аргументов командной строки
while [ "$1" != "" ]; do
  case $1 in
    -c | --cmd )            shift
			    command=$1
                            ;;
    -i | --input )    	    standart_input=1
                            ;;
    -t | --threads )	    shift
		            num_threads_in_parallel=$1
			    ;;
    -h | --help )           usage
                            exit
                            ;;
    * )                     if [ ! -z "$file" ]; then
			      echo "Only one file can be passed as positional argument (multiple files: $file, $1)"
			      usage
		            exit				
			    fi
			    file=$1
                            ;;
  esac
  shift
done

#echo 'File ' $file
#echo 'Command ' $command
#echo 'Input flag ' $standart_input
#echo 'Threads '$num_threads_in_parallel

if [ -z "$file" ]; then
  echo "Input file should be specified"
  usage
  exit
fi

# Проверка существования входного файла
if [ ! -f "$file" ]; then
  echo "File $file doesn't exist"
  exit
fi
# Проверка корректности количества параллельно исполняемых команд
if ! [ "$num_threads_in_parallel" -ge 1 2>/dev/null ]; then
  echo "Threads parameter '$num_threads_in_parallel' is not valid"
  usage
  exit
fi

# Формирование строки, которую необходимо будет выполнять (например, 'echo ' или 'echo < ')
command_prefix=
if [ $standart_input -eq 0 ]; then
  command_prefix=$command' '
else
  command_prefix=$command' < '
fi

concatenated_commands_count=0
command_to_eval=

# Читаем все строки подряд, с помощью них формируем исполняемые команды и приклеиваем к переменной command_to_eval
# Когда счетчик приклеенных команд достигает значения num_threads_in_parallel, выполняем команду
# В случае паралеллельного исполнения одной копии, команды будут вида 'echo текущая_строка & wait'
# В случае паралеллельного исполнения двух копий, команды будут вида 'echo предыдущая_строка & echo текущая_строка & wait'
# и так далее
while IFS='' read -r line; do
  if [ $concatenated_commands_count -gt 0 ]; then
    command_to_eval="$command_to_eval & "
  fi
  command_to_eval="$command_to_eval$command_prefix\"$line\""
  concatenated_commands_count=$((concatenated_commands_count+1))
  if [ $concatenated_commands_count -eq "$num_threads_in_parallel" ]; then
    eval "$command_to_eval & wait"
    concatenated_commands_count=0
    command_to_eval=
  fi
done < "$file"
# Если количество строк не делится нацело на количество параллельно исполняемых команд, 
# то последние команды не будут выполнены в цикле, поэтому выполняем их отдельно
if [ ! -z "$command_to_eval" ]; then
  eval "$command_to_eval & wait"
fi
