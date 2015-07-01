--
-- Usage: lua main.lua PIPELINE_STRING
--     luajit main.lua 'videotestsrc ! ximagesink name=vsink'

local lgi  = require 'lgi'
local glib = lgi.GLib
local gtk  = lgi.require('Gtk')
local gst  = lgi.Gst
local log  = lgi.log.domain('my-gst-app')

local pipeline, valve, vsink

gtk.init()
gst.init()

local app = gtk.Application { application_id = 'net.kalio.gstreamer.test' }
local win = gtk.Window { title = 'gstreamer test app',
                         window_position = 'CENTER',
                         default_width = 400, default_height = 300, 
                         on_destroy=gtk.main_quit }

local button_state = false
local button  = gtk.Button { label = string.format('Toggle') }
function button:on_clicked ()
   log.message("button pressed!")
   button_state = not button_state
   if button_state then
      pipeline.state = 'PLAYING'
   else
      pipeline.state = 'PAUSED'
   end
end
local quit_button = gtk.Button { label = 'Quit' }
function quit_button:on_clicked ()
   log.message("quit button pressed")
   pipeline.state = 'NULL'
   app:quit()
end

local vbox = gtk.VBox()
vbox:pack_start(button, false, false, 0)
vbox:pack_start(quit_button, false, false, 0)
win:add(vbox)

local pipedefstr = arg[1] or "videotestsrc ! ximagesink"
pipeline = gst.parse_launch(pipedefstr)
-- valve = pipeline:get_by_name('valve0')
-- vsink = pipeline:get_by_name('vsink')

if not valve then
   log.warning('Element valve0 not found in the pipeline')
end

function app:on_activate()
   log.message('app activated')
   win.application = app
   win:show_all()
   pipeline.state = 'PLAYING'
end

app:run() -- { arg[1], ... }
log.message('main loop terminated')



