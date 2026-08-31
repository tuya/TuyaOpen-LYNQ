#coding=utf-8
import web
import os
from web import form
import json
import random
import requests

urls = (
	'/', 'index',
	'/config','config'
)

app = web.application(urls, globals())
render = web.template.render('templates/')

myform = form.Form(
	form.Textbox("boe"),
	form.Textbox("bax",
		form.notnull,
		form.regexp('\d+', 'Must be a digit'),
		form.Validator('Must be more than 5', lambda x:int(x)>5)),
	form.Textarea('moe'),
	form.Checkbox('curly'),
	form.Dropdown('french', ['mustard', 'fries', 'wine']))

def send_post_to_onenet(id):
    tts_str = "支付宝收款" + str(round(random.uniform(0, 10000), 2)) + "元"
    tts_cmd = {"name":"systestPlayTts","param1":0,"param2":0,"param3":tts_str}
    onenet_url    = "http://api.heclouds.com/cmds?device_id=" + id
    onenet_header = {"api-key":"TWFwVbgRG5lg6H8J=B=w1TcqBbI="}
    response = requests.post(url=onenet_url, headers=onenet_header, data=json.dumps(tts_cmd,sort_keys=True))
    print('')
    print(tts_cmd)
    print(response.text)

class index:
	def GET(self):
		send_post_to_onenet(web.input().id)
		return render.index()

class config:
	def GET(self):
		form = myform()
		# make sure you create a copy of the form by calling it (line above)  
		# Otherwise changes will appear globally  
		print(form.render())
		return render.config(form) 
		
	def POST(self): 
			form = myform()
			if not form.validates():
				print(form.render())
				return render.config(form)
			else:
				# form.d.boe and form['boe'].value are equivalent ways of
				# extracting the validated arguments from the form.
				return "Grrreat success! boe: %s, bax: %s" % (form.d.boe, form['bax'].value)

if __name__ == "__main__":
	print("auto test web server start")
	web.config.debug=False
	# os.popen('jupyter notebook --NotebookApp.port=8866')
	app.run()