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
FILE=
# Команда, которая будет выполняться для строк из файлы
COMMAND=echo
# Передача строки как аргумента или на стандартный вход
STANDART_INPUT=0
# Количество копий команды, запускаемых одновременно
NUM_THREADS_IN_PARALLEL=1

# Парсинг аргументов командной строки
while [ "$1" != "" ]; do
  case $1 in
    -c | --cmd )            shift
			    COMMAND=$1
                            ;;
    -i | --input )    	    STANDART_INPUT=1
                            ;;
    -t | --threads )	    shift
		            NUM_THREADS_IN_PARALLEL=$1
			    ;;
    -h | --help )           usage
                            exit
                            ;;
    * )                     if [ ! -z "$FILE" ]; then
			      echo "Only one file can be passed as positional argument (multiple files: $FILE, $1)"
			      usage
		            exit				
			    fi
			    FILE=$1
                            ;;
  esac
  shift
done

#echo 'File ' $FILE
#echo 'Command ' $COMMAND
#echo 'Input flag ' $STANDART_INPUT
#echo 'Threads '$NUM_THREADS_IN_PARALLEL

if [ -z "$FILE" ]; then
  echo "Input file should be specified"
  usage
  exit
fi

# Проверка существования входного файла
if [ ! -f "$FILE" ]; then
  echo "File $FILE doesn't exist"
  exit
fi
# Проверка корректности количества параллельно исполняемых команд
if ! [ "$NUM_THREADS_IN_PARALLEL" -ge 1 2>/dev/null ]; then
  echo "Threads parameter '$NUM_THREADS_IN_PARALLEL' is not valid"
  usage
  exit
fi

# Формирование строки, которую необходимо будет выполнять (например, 'echo ' или 'echo < ')
COMMAND_PREFIX=
if [ $STANDART_INPUT -eq 0 ]; then
  COMMAND_PREFIX=$COMMAND' '
else
  COMMAND_PREFIX=$COMMAND' < '
fi

CONCATENATED_COMMANDS_COUNT=0
COMMAND_TO_EVAL=

# Читаем все строки подряд, с помощью них формируем исполняемые команды и приклеиваем к переменной COMMAND_TO_EVAL
# Когда счетчик приклеенных команд достигает значения NUM_THREADS_IN_PARALLEL, выполняем команду
# В случае паралеллельного исполнения одной копии, команды будут вида 'echo текущая_строка & wait'
# В случае паралеллельного исполнения двух копий, команды будут вида 'echo предыдущая_строка & echo текущая_строка & wait'
# и так далее
while IFS='' read -r line; do
  if [ $CONCATENATED_COMMANDS_COUNT -gt 0 ]; then
    COMMAND_TO_EVAL="$COMMAND_TO_EVAL & "
  fi
  COMMAND_TO_EVAL="$COMMAND_TO_EVAL$COMMAND_PREFIX\"$line\""
  CONCATENATED_COMMANDS_COUNT=$((CONCATENATED_COMMANDS_COUNT+1))	
  if [ $CONCATENATED_COMMANDS_COUNT -eq "$NUM_THREADS_IN_PARALLEL" ]; then
    eval "$COMMAND_TO_EVAL & wait"
    CONCATENATED_COMMANDS_COUNT=0
    COMMAND_TO_EVAL=
  fi
done < "$FILE"
# Если количество строк не делится нацело на количество параллельно исполняемых команд, 
# то последние команды е будут выполнены в цикле, поэтому выполняем их отдельно
if [ -n "$COMMAND_TO_EVAL" ]; then
  eval "$COMMAND_TO_EVAL & wait"
fi
