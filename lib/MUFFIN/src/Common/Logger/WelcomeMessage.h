/**
 * @file WelcomeMessage.h
 * @author Moon, Sun-hwa (woowsnu@edgecross.ai)
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 시스템 부팅 시 시리얼 모니터에 출력하는 웰컴 메시지입니다.
 * 
 * @date 2023-10-21
 * @version 1.0.0
 * 
 * @copyright Copyright (c)Edgecross inc. 2024
 */




#pragma once

#include <pgmspace.h>



const char PROGMEM welcomAsciiArt[] =
"\r\n"
"\r\n"
"      ;;.\r\n"
"     :x+;.\r\n"
"    :+xx+;\r\n"
"    :+xxx+:\r\n"
"     :+xxx+:\r\n"
"      :+xxx+:               ::+:        \r\n"
"       ;+xxx+:       .:;++++++:       ############## ##############   ##############  ############# \r\n"
"       .;+xxx+:::;;+++++++x++:        ############## ##           ## ##       #####  ############## \r\n"
"        .;+xxxxx++++++++++;;:.        ##             ##           ## ##          ##  ##\r\n"
"         .;+xx+xx+++;::.              #############  ##############   #############  #############\r\n"
"          .;xxxx+;:.                  \r\n"
"             ...:::             ###########  #############     #########      #############   ############       \r\n"
"           .:;xxxxx+:        ###             ##          ##  ##         ##   ##              ##\r\n"
"     ..:;;;++++++xxx+:       ##              ##     #######  #           ##  #############   #############\r\n"
"  ;;;++x+++;x+++xxxxx+:      ###             ##       ###    ###        ##             ##              ##\r\n"
" :++;+++++x;::.. ;+xxx+:       ############  ##         ##      #######      ###########     ###########         \r\n"
":++x+;::.         ;xxxx+:      \r\n"
"::.                ;xxxx+:     \r\n"
"                   .;+xxx+:             \r\n"
"                    .;+xxx+:                                                                LSJ, KJS, MSH\r\n"
"                     .;+xx;                                                     Firmware Development Team\r\n"
"                      .+x;                                              Copyright Edgecross Inc. (c) 2023\r\n"
"                       .;.\r\n"
"\r\n";