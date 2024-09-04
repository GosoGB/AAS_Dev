/**
 * @file GreetingMessage.h
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



const char PROGMEM welcomAsciiArt[] = R"(


      ;;.
     :x+;.
    :+xx+;
    :+xxx+:
     :+xxx+:
      :+xxx+:               ::+:        
       ;+xxx+:       .:;++++++:       ############## ##############   ##############  ############# 
       .;+xxx+:::;;+++++++x++:        ############## ##           ## ##       #####  ############## 
        .;+xxxxx++++++++++;;:.        ##             ##           ## ##          ##  ##
         .;+xx+xx+++;::.              #############  ##############   #############  #############
          .;xxxx+;:.                  
             ...:::             ###########  #############     #########      #############   ############       
           .:;xxxxx+:        ###             ##          ##  ##         ##   ##              ##
     ..:;;;++++++xxx+:       ##              ##     #######  #           ##  #############   #############
  ;;;++x+++;x+++xxxxx+:      ###             ##       ###    ###        ##             ##              ##
 :++;+++++x;::.. ;+xxx+:       ############  ##         ##      #######      ###########     ###########         
:++x+;::.         ;xxxx+:      
::.                ;xxxx+:     
                   .;+xxx+:             
                    .;+xxx+:                                                                LSJ, KJS, MSH
                     .;+xx;                                                     Firmware Development Team
                      .+x;                                              Copyright Edgecross Inc. (c) 2023
                       .;.






)";