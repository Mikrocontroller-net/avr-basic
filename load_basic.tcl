#
# geht noch nicht!!!!!!!!!!!!!!!!!!
#
#
#
#
#
package require Tk

set gvar(com) /dev/ttyUSB0
set gvar(mode) "57600,n,8,1"
set gvar(prog_dir) basic/
set gvar(akt_prog) ""

#***********************************************
proc read_com {} {
	global gvar
	if {[gets $gvar(chan) line] > 0} {
		.f2.f21.console configure -state normal
		.f2.f21.console insert {end - 1 char} $line\n
		.f2.f21.console configure -state disable
		.f2.f21.console see end
	}
}

#***********************************************
proc gui {} {
	global gvar
	wm title . "uBasic-avr $gvar(akt_prog)" 
	wm resizable . 0 0
	pack [labelframe .f1 -text "uBasic:"] -fill x -padx 5
	pack [frame .f1.f11] -fill x -padx 5 -side left -fill y
	pack [frame .f1.f12] -fill x -padx 5 -side right -fill y
	pack [frame .f1.f12.f121] -fill x -padx 5 -side top -fill y
	pack [frame .f1.f12.f122] -fill x -padx 5 -side top -fill y
	
	pack [label .f1.f11.akt_prog -textvar gvar(akt_prog)] -side top
	
	text .f1.f11.basic -height 24 -width 80
	scrollbar .f1.f11.sb -command [list .f1.f11.basic yview]
	.f1.f11.basic configure -yscrollcommand [list .f1.f11.sb set]
	pack .f1.f11.basic .f1.f11.sb -side left -expand 1 -fill both
	
	pack [label .f1.f12.f121.l1 -text "Programs:"] -fill x -anchor n
	listbox .f1.f12.f121.progs -height 10 -width 15
	scrollbar .f1.f12.f121.sb -command [list .f1.f12.f121.progs yview]
	.f1.f12.f121.progs configure -yscrollcommand [list .f1.f12.f121.sb set]
	pack .f1.f12.f121.progs .f1.f12.f121.sb -side left -expand 1 -fill both
	
	bind .f1.f12.f121.progs <<ListboxSelect>> {prog_select [selection get]}

	
	pack [button .f1.f12.f122.new -text "new" -width 15 -command prog_new] -fill x -side top
	pack [button .f1.f12.f122.save -text "save" -width 15 -command prog_save] -fill x -side top
	pack [button .f1.f12.f122.transfer -text "transfer" -width 15 -command send_basic] -fill x -side top
	pack [label .f1.f12.f122.l1 -text ""] -fill x -side top
	pack [button .f1.f12.f122.delete -text "delete" -width 15 -command prog_delete] -fill x -side bottom
	
	
	pack [labelframe .f2 -text "Konsole"] -fill x -padx 5
	pack [frame .f2.f21] -fill x -padx 5 -side left -fill y
	pack [frame .f2.f22] -fill x -padx 5 -side right -fill y

	text .f2.f21.console -height 24 -width 80  -state disable
	scrollbar .f2.f21.sb -command [list .f2.f21.console yview]
	.f2.f21.console configure -yscrollcommand [list .f2.f21.sb set]
	pack .f2.f21.console .f2.f21.sb -side left -expand 1 -fill both

	pack [label  .f2.f22.l1 -text "Com-Port:"] -fill x -side top
	pack [entry  .f2.f22.com -width 15 -textvar gvar(com)] -fill x -side top
	pack [label  .f2.f22.l4 -text "Mode:"] -fill x -side top
	pack [entry  .f2.f22.com_mode -width 15 -textvar gvar(mode)] -fill x -side top
	pack [label  .f2.f22.l2 -text ""] -fill x -side top
	pack [label  .f2.f22.l3 -text "Commands:"] -fill x -side top
	pack [button .f2.f22.connect -text "connect" -width 15 -command connect] -fill x -side top
	pack [button .f2.f22.list -text "list" -width 15 -command {send_cmd "list"} -state disable] -fill x -side top
	pack [button .f2.f22.run -text "run" -width 15 -command {send_cmd "run"} -state disable] -fill x -side top
	pack [button .f2.f22.break -text "break" -width 15 -command {send_cmd "break"} -state disable] -fill x -side top
	pack [button .f2.f22.clear -text "clear" -width 15 -command clear_console] -fill x -side top
	pack [button .f2.f22.disconnect -text "disconnect" -width 15 -command disconnect -state disable] -fill x -side top
	pack [button .f2.f22.exit -text "exit" -command exit -width 15	] -fill x -side bottom
}

#***********************************************
proc load_prog_names {} {
	global gvar	
	.f1.f12.f121.progs delete 0 end
	set dir [glob "$gvar(prog_dir)*.bas"]
	set dir [lsort $dir]
	foreach {f} $dir {
		.f1.f12.f121.progs insert end [file tail $f]
	}
}

#***********************************************
proc prog_select {name} {
	global gvar
	.f1.f11.basic delete 1.0 end
	set gvar(akt_prog) $gvar(prog_dir)$name
	set file [open $gvar(akt_prog)]
	while {[gets $file line] >= 0} {.f1.f11.basic insert end "$line\n"}
	close $file
}

#***********************************************
proc prog_save {} {
	global gvar
	if {$gvar(akt_prog)!=""} {
		set basic_txt [.f1.f11.basic get 1.0 end]
		set file [open $gvar(akt_prog) w]
		puts $file $basic_txt
		close $file
		load_prog_names
	}
}

#***********************************************
proc prog_new {} {
	global gvar
	set gvar(akt_prog) [tk_getSaveFile]
	.f1.f11.basic delete 1.0 end
	prog_save
	load_prog_names
}

#***********************************************
proc prog_delete {} {
	global gvar
	if {[tk_messageBox -type yesno -title "Frage..." -icon question -message "Wirklich loeschen?"]} {
		file delete $gvar(akt_prog)
		load_prog_names
		.f1.f11.basic delete 1.0 end
		set gvar(akt_prog) ""
	}
}

#***********************************************
proc connect {} {
	global gvar
	set gvar(chan) [open $gvar(com) RDWR]
	fconfigure $gvar(chan) -mode $gvar(mode)
	
	#fconfigure $gvar(chan) -blocking 1 -buffering full -translation binary
	fileevent $gvar(chan) readable read_com
	.f2.f22.connect configure -state disable
	.f2.f22.disconnect configure -state normal
	.f2.f22.run configure -state normal
	.f2.f22.list configure -state normal
}

#***********************************************
proc send_cmd {cmd} {
	global gvar
	puts $gvar(chan) "$cmd\r\n"
	flush $gvar(chan)
	if {$cmd=="run"} {
		.f2.f22.break configure -state normal
		.f2.f22.list configure -state disable
	}
	if {$cmd=="break"} {
		.f2.f22.run configure -state normal
		.f2.f22.list configure -state normal
		.f2.f22.break configure -state disable
	}
}

#***********************************************
proc send_basic {} {
	global gvar
	puts $gvar(chan) "load\n"
	flush $gvar(chan)	
	set basic_txt [.f1.f11.basic get 1.0 end]
	puts $gvar(chan) -nonewline $basic_txt
	flush $gvar(chan)	
	puts $gvar(chan) "#"
	flush $gvar(chan)	
}


#***********************************************
proc disconnect {} {
	global gvar
	close $gvar(chan)
	.f2.f22.connect configure -state normal
	.f2.f22.disconnect configure -state disable
	.f2.f22.run configure -state disable
	.f2.f22.list configure -state disable
	.f2.f22.break configure -state disable
}

#***********************************************
proc clear_console {} {
	global gvar
	.f2.f21.console configure -state normal
	.f2.f21.console delete 1.0 end
	.f2.f21.console configure -state disable
}

#***********************************************
#***********************************************
#***********************************************

gui
load_prog_names

#set chan [open $gvar(com) RDWR]
#fconfigure $chan -blocking 1 -buffering full -translation binary
#fileevent $chan readable read_com




#puts $chan "list\r\n"
#flush $chan
#after 1000 output
#vwait forever
#close $chan
