import telebot; #library for use telegram
import subprocess; 
import os;
import socket; #lib for connect to server database
import time; # lib for use sleep()
from telebot import types 

bot=telebot.TeleBot('5359726644:AAHGXDYUF0fj__G0pdpgLluCPnHFIO0Wcpk'); #token your bot
port=4000 # port server database	
ipaddr="127.0.0.1" #address server database

@bot.message_handler(content_types=['text'])
def start(message):
	'''
	if(allow_write==1):
		req=message.text
		if(req[0]=="1"):
			bot.send_message(message.from_user.id,"hi")
	'''
	if message.text=="/start":
		showkeyinkeyboard(message)
	elif message.text =="/help" :
		hmes="Command\n/help - this menu\n/sen - sensors view\n /getmem - show button memory\n /ret - return to one page menu\n/getmem_l - get last minute memory stat\n /getmem_l10 - Get last 10 minute value\n /getmem_a - Get all database !!!\n/getmem_cus - Get custom request to database\n\tExample\nselect m_total, m_free, m_cached, m_available, t_sec,t_min,t_hour,t_day,t_mon,t_year from memstat where t_min='23' order by t_day DESC\0"
		bot.send_message(message.from_user.id,hmes)
		showkeyinkeyboard(message)
	elif message.text =="/sen" or message.text =="Get sensors":
		rez=subprocess.check_output(["sensors"])
		bot.send_message(message.from_user.id,rez)
	elif message.text =="/ret" or message.text =="Return to main menu":
		showkeyinkeyboard(message)
	elif message.text =="/getmem" or message.text =="Get memory stat":
		showkeymemory(message)
	elif message.text =="/getmem_l" or message.text =="Get last memory stat":
		getlastmemory(message,1,"")
	elif message.text =="/getmem_l10" or message.text =="Get last 10 value":
		getlastmemory(message,2,"")
	elif message.text =="/getmem_a" or message.text =="Get all database":
		getlastmemory(message,3,"")
	elif message.text =="Get last all stat":
		getlastmemory(message,1,"")
		rez=subprocess.check_output(["sensors"])
		bot.send_message(message.from_user.id,rez)
	elif message.text =="/getmem_cus" or message.text =="Get custom request to database":
		bot.send_message(message.from_user.id,"Enter request\n Example\nselect m_total, m_free, m_cached, m_available, t_sec,t_min,t_hour,t_day,t_mon,t_year from memstat where t_min='23' order by t_day DESC")
		bot.register_next_step_handler(message,entercustomrequest);
	else:
		bot.send_message(message.from_user.id,"I am not undastand you! write /help to show command or tap to button")
		showkeyinkeyboard(message)
def entercustomrequest(message):
	reqstr=message.text
	getlastmemory(message,4,reqstr)
def showkeymemory(message):
	keyboard=types.ReplyKeyboardMarkup(resize_keyboard=True);
	buttons=["Get last memory stat","Get last 10 value"]
	buttons1=["Get custom request to database","Get all database"]
	buttons2=["Return to main menu"]
	keyboard.add(*buttons);
	keyboard.add(*buttons1);
	keyboard.add(*buttons2);
	bot.send_message(message.from_user.id,text="Choise",reply_markup=keyboard)
def showkeyinkeyboard(message):
	keyboard=types.ReplyKeyboardMarkup(resize_keyboard=True);
	buttons=["Get memory stat","Get sensors"]
	key_gm2=types.InlineKeyboardButton(text="Get last all stat");
	keyboard.add(*buttons)
	keyboard.add(key_gm2);
	bot.send_message(message.from_user.id,text="Choise",reply_markup=keyboard)
def getlastmemory(message,t,reqstr):
	try:
		sock=socket.socket(socket.AF_INET,socket.SOCK_STREAM)
	except socket.error as err :
		print("Error create socket")
	sock.connect((ipaddr,port))
	
	
	#request="select m_total, m_free, m_cached, m_available, t_sec,t_min,t_hour,t_day,t_mon,t_year from memstat where t_min='23' order by t_day DESC \0"
	day=time.strftime("%d")
	minuts=time.strftime("%M")
	hour=time.strftime("%H")
	year=time.strftime("%Y")
	month=time.strftime("%m")
	minuts=int(minuts)
	if t==1:
		request=f"select m_total, m_free, m_cached, m_available, t_sec,t_min,t_hour,t_day,t_mon,t_year from memstat where t_min='{minuts-1}' AND t_hour='{hour}' AND t_day='{day}' AND t_mon='{month}' AND t_year='{year}'\0"
		print(request)
	elif t==2:
		request=f"select m_total, m_free, m_cached, m_available, t_sec,t_min,t_hour,t_day,t_mon,t_year from memstat where t_min>'{minuts-11}'AND t_min<'{minuts}' AND t_hour='{hour}' AND t_day='{day}' AND t_mon='{month}' AND t_year='{year}'\0"
		print(request)
	
	elif t==3:
		request="select m_total, m_free, m_cached, m_available, t_sec,t_min,t_hour,t_day,t_mon,t_year from memstat\0"
	else:
		request=reqstr
	sock.sendall(bytes(request,'utf-8'))
	time.sleep(1)
	data=sock.recv(1024)
	while(len(data)>1):
	
		bot.send_message(message.from_user.id,data)
		data=sock.recv(1024)
	if len(data)<2:
		data="Low len"
	sock.close()
bot.polling(none_stop=True,interval=0)
