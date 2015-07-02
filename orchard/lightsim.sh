#!/bin/bash
clear
tput civis
while true
do
  red=$(cat /sys/class/gpio/gpio6/value)
  yellow=$(cat /sys/class/gpio/gpio13/value)
  green=$(cat /sys/class/gpio/gpio19/value)

  if [ ${red} -eq 1 ]
  then
    echo -n -e "\e[1;29;41m${red}\e[0m"
  else
    echo -n -e "\e[1;31;40m${red}\e[0m"
  fi

  if [ ${yellow} -eq 1 ]
  then
    echo -n -e "\e[1;29;43m${yellow}\e[0m"
  else
    echo -n -e "\e[1;33;40m${yellow}\e[0m"
  fi

  if [ ${green} -eq 1 ]
  then
    echo -n -e "\e[1;30;42m${green}\e[0m"
  else
    echo -n -e "\e[1;32;40m${green}\e[0m"
  fi

  sleep .2
  printf "\r"
done
