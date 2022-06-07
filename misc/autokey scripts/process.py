import os
import time
import shutil
import keyboard as kb


# alt: is process open?
def is_window_active(name):
    # Should be a regex
    active = window.get_active_title()
    if active.startswith(name):
        return True
    return False
    
def msg_box(msg, title='Message'):
    #winTitle = window.get_active_title()
    dialog.info_dialog(title=title, message=msg)
    

def wait_for_window(name, attempts=1, interval=1):
    while attempts > 0:
        if is_window_active(name):
            return True
        elif attempts > 1:
            time.sleep(interval)
        attempts -= 1
    return False
    
def wait_for_window_close(name, attempts=1, interval=1):
    while attempts > 0:
        if not is_window_active(name):
            return True
        elif attempts > 1:
            time.sleep(interval)
        attempts -= 1
    return False
    

def start_ida(ida_path, filename):
    system.exec_command(ida_path + ' ' + filename, getOutput=False)    
    
def ida_handle_loadscreen():
    exe_type = ""
    tries = 0
    while not exe_type.lower().startswith("Portable executable".lower()):
        keyboard.send_keys("<ctrl>+c")
        time.sleep(0.5)
        exe_type = clipboard.get_clipboard()
        tries += 1
        if tries > 3:
            keyboard.send_keys("<escape>")
            return False
        
    keyboard.send_keys("<enter>")
    return True
    
def ida_handle_analysis(log_file, attempts=3, interval=0.5):
    while True:
        active = window.get_active_title()
        if active.startswith("IDA"):
            text = ida_get_output(0.5)
            if "The initial autoanalysis has been finished." in text:
                return True
            else:
                keyboard.send_keys("<escape>")
        elif active.startswith("Select file for module"):
            keyboard.send_keys("<escape>")
        elif active.startswith("Warning"):
            keyboard.send_keys("<escape>")
        elif active.startswith("Please confirm"):
            keyboard.send_keys("<enter>")
        elif active.startswith("Information"):
            keyboard.send_keys("<enter>")
        elif active.startswith("gnome-shell"):
            log(log_file, 'Got gnome bug during analysis')
            window.activate("IDA.*")
            time.sleep(0.2)
            window.activate("Select file for module")
            time.sleep(0.2)
            keyboard.send_keys("<escape>")
            #return False
        elif active.startswith("Please wait..."):
            #msg_box("please wait bug")
            log(log_file, 'Got Please Wait bug during analysis')
            attempts -= 1
            #return False
        elif attempts > 0:
            attempts -= 1
        if attempts == 0:
            return False
        time.sleep(interval)
    
    return False
    #finished = False
    #while not finished:
    #   finished = ida_get_output(0.5)
        
        
def ida_get_output(interval=0.2):
    keyboard.send_keys("<alt>+0")
    time.sleep(interval)
    keyboard.send_keys("<ctrl>+a")
    time.sleep(interval)
    keyboard.send_keys("<ctrl>+c")
    time.sleep(interval)
    #text = clipboard.get_selection()
    text = clipboard.get_clipboard()
    time.sleep(interval*5)
    return text

    
    
def ida_export_asm(wait_time=0.5, attempts=20):
    keyboard.send_keys("<alt>+<f10>")
    time.sleep(wait_time*2)
    keyboard.send_keys("<enter>")
    while attempts > 0:
        text = ida_get_output()
        if "Assembler file has been created, total" in text:
            return True
        else:
            time.sleep(wait_time)
            attempts -= 1
    return False
    
    
def ida_exit(wait_time=0.5):
    keyboard.send_keys("<alt>+1")
    time.sleep(wait_time)
    keyboard.send_keys("<alt>+x")
    time.sleep(wait_time)
    keyboard.send_keys("<tab>")
    time.sleep(wait_time)
    keyboard.send_keys("<tab>")
    time.sleep(wait_time)
    keyboard.send_keys(" ")
    time.sleep(wait_time)
    keyboard.send_keys("<enter>")



    
def log(fname, msg):
    with open(fname, 'a') as f:
        f.write(str(msg) + '\n')
    
    
    
def process_file(file, exe_dir, asm_dir, log_file, ida_path):
    start_ida(ida_path, exe_dir + file)
    
    started = wait_for_window('Load a new file', attempts=3)
    if not started:
        log(log_file, 'FAIL/COULD NOT REACH LOAD SCREEN: ' + file)
        return
        
    loading = ida_handle_loadscreen()
    if not loading:
        log(log_file, 'FAIL/STUCK AT LOAD SCREEN: ' + file)
    
    time.sleep(1) # Wait for 1s to bypass confirmation issues
    
    analyzed = ida_handle_analysis(log_file, attempts=3)
    if not analyzed:
        log(log_file, 'FAIL/COULD NOT OPEN/FINISH ANALYSIS: ' + file)
        log(log_file, window.get_active_title())
        return
        
    time.sleep(0.5)
    #clipboard.fill_clipboard('')
    '''text = ida_get_output(0.5)
    if "The initial autoanalysis has been finished." not in text:
        log(log_file, 'FAIL/ANALYSIS DOUBLE CHECK FAILED: ' + file)
        return'''
        
    saved = ida_export_asm(wait_time=0.5, attempts=120)
    if not saved:
        log(log_file, 'FAIL/COULD NOT SAVE ASM: ' + file)
        log(log_file, window.get_active_title())
        return
    if not os.path.exists(exe_dir + file + '.asm'):
        log(log_file, 'FAIL/ASM FILE NOT GENERATED: ' + file)
        return
               
    time.sleep(0.2) #workaround i dont understand
    ida_exit(wait_time=0.5)
    time.sleep(0.2)
    
    closed = wait_for_window_close("IDA", attempts=3, interval=1)
    if not closed:
        log(log_file, 'FAIL/COULD NOT CLOSE IDA: ' + file)
        return
    
    shutil.move(exe_dir + file + '.asm', asm_dir)
    if not os.path.exists(asm_dir + file + '.asm'):
        log(log_file, 'FAIL/ASM FILE NOT FOUND: ' + file)
        return

    log(log_file, 'SUCCESS: ' + file)    



def cleanup(log_file):
    try:
        ida_running = window.wait_for_exist('IDA.*', timeOut=0.1)
        active = window.get_active_title()
        if ida_running:
            if active.startswith("Select file for module"):
                keyboard.send_keys("<escape>")
            elif active.startswith("Warning"):
                keyboard.send_keys("<escape>")
            elif active.startswith("Please confirm"):
                keyboard.send_keys("<enter>")
            elif active.startswith("Information"):
                keyboard.send_keys("<enter>")
            elif active.startswith("Please wait..."):
                log(log_file, 'Please wait bug')
                keyboard.send_keys("<escape>")
                time.sleep(0.2)
                window.activate("Select file for module")
                time.sleep(0.2)
                keyboard.send_keys("<escape>")
            elif not active.startswith("IDA"):
                keyboard.send_keys("<escape>")
                keyboard.send_keys("<enter>")
            time.sleep(0.1)
            active = window.get_active_title()
            if active.startswith('IDA'):
                ida_exit()
                time.sleep(0.5)
            ida_running = window.wait_for_exist('IDA.*', timeOut=0.1)
            if ida_running:
                log(log_file, 'COULD NOT CLEAN STATE.')
                return False
    except:
        log(log_file, 'ERROR WHEN CLEARING.')
        return False
    
    return True


def main():
    family_file = '/home/meh/Desktop/malware/families.txt'
    num_families = 20
    list_file_str = '/home/meh/Desktop/malware/malware_files/{:s}.txt'
    exe_dir_str = '/home/meh/Desktop/malware/exe/{:s}/'
    log_file_str = '/home/meh/Desktop/malware/logs/{:s}_log.txt'
    asm_dir_str = '/home/meh/Desktop/malware/asm/{:s}/'
    ida_path = '/home/meh/idafree-7.7/ida64'
    suffix = ''
    
    families = []
    with open(family_file, 'r') as f:
        families = [line.strip() for line in f if line.strip() != '']
    families = families[0:num_families]
    
    for family in families[12:20]:
        list_file = list_file_str.format(family)
        exe_dir = exe_dir_str.format(family)
        asm_dir = asm_dir_str.format(family)
        log_file = log_file_str.format(family.lower())
        
        # Reset log file for this family
        with open(log_file, 'w') as f: 
            pass
        
        if not os.path.isdir(asm_dir):
            os.makedirs(asm_dir)
    
        files = []
        with open(list_file, 'r') as f:
            files = [line.strip() + suffix for line in f if line.strip() != '']
        
        for file in files:
            # Skip if already done
            if os.path.exists(asm_dir+file+'.asm'):
                log(log_file, 'SKIP: ' + file)
                continue
        
            try:
                process_file(file, exe_dir, asm_dir, log_file, ida_path)
            except:
                pass
        
            
            attempts = 3
            cleaned = cleanup(log_file)
            while not cleaned and attempts > 0:
                cleaned = cleanup(log_file)
                if store.GLOBALS.get("STOP", False):
                    store.set_global_value("STOP", False)
                    exit()
                attempts -= 1
                time.sleep(3)
            
            if store.GLOBALS.get("STOP", False):
                store.set_global_value("STOP", False)
                exit()
                
            time.sleep(0.5)
    
    
main()