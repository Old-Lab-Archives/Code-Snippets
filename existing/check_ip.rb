# A script to check your IP address and email you when it changes

require 'uri'
require 'yaml'
require 'json'
require 'net/http'

class IpCheck
  CONFIG_DIR = "#{ENV['HOME']}/.ip-check"
  CONFIG_FILE = "config.yml"
  CONFIG_DEFAULT = <<JSON
send_ok: false
send_to: receiver@example.com
send_from: sender@example.com
subject: IP Check
JSON
  LAST_IP_FILE = 'last_ip'
  LOG_FILE = 'log'
  MESSAGE_QUEUE = 'queue'

  def initialize
    @config = get_config
    puts @config
  end

  def get_ip
    json_ip = Net::HTTP::get URI('http://jsonip.com')
    ip_data = JSON::parse(json_ip)
    ip_data['ip']
  end

  def check_config_dir
    config_dir = CONFIG_DIR
    if !File.exists? config_dir
      Dir.mkdir config_dir
    elsif !File.directory?(config_dir)
      raise "File #{config_dir} already exists."
    end
    true
  end

  def get_config
    check_config_dir # ensure config directory exists
    config_file_path = File.join(CONFIG_DIR, CONFIG_FILE)
    if !File.exists?(config_file_path)
      File.open(config_file_path, 'w') do |fh|
        fh.write get_config_default
      end
    end
    File.open config_file_path do |fh|
      YAML::load fh.read
    end
  end

  def get_config_default
    unless STDIN.tty?
      raise "Cannot write defaults from non-tty"
    end

    sender = nil
    receiver = nil

    loop do
      puts "Who to send from?"
      sender = gets.chomp
      break if sender.match /^\S+?@\S+?.\S+?$/
    end 

    loop do
      puts "Who to send to?"
      receiver = gets.chomp
      break if receiver.match /^\S+?@\S+?.\S+?$/
    end

    CONFIG_DEFAULT.gsub('sender@example.com', sender).gsub('receiver@example.com', receiver)
  end

  def check_ip
    last_ip_path = File.join(CONFIG_DIR, LAST_IP_FILE)
    begin
      ip = get_ip

      if File.exists?(last_ip_path)
        last_ip = File.read(last_ip_path)
        remember_ip ip
        if last_ip == ip
          send_ok
          write_log 'OK'
        else
          send_changed ip
          write_log "IP Changed: #{ip}"
        end
      else
        remember_ip ip
        send_welcome ip
        write_log "Welcome, your current IP address is #{ip}"
      end
    rescue => e
      write_log e.message
      raise e
    end
  end

  def remember_ip ip
    last_ip_path = File.join(CONFIG_DIR, LAST_IP_FILE)
    File.open(last_ip_path, 'w') do |fh|
      fh.write ip
    end
  end

  def send_ok
    if @config['send_ok']
      send_email
    end
  end

  def send_welcome ip
    send_email "Welcome, your IP address is #{ip}"
  end

  def send_changed ip
    send_email "Your IP has changed to #{ip}"
  end

  def send_email message
    IO::popen(['sendmail', '-f', @config['send_from'], @config['send_to']], "w+") do |sm|
      sm.puts <<EMAIL
Subject: #{@config['subject']}
#{message}
EMAIL
      sm.flush
    end
  end

  def write_log message
    log_file = File.join(CONFIG_DIR, LOG_FILE)
    File.open log_file, 'a' do |fh|
      fh.write("#{Time.now.to_s}\t--\t#{message}\n")
    end
    if STDOUT.tty?
      puts message
    end
  end

  def self.run!
    instance = IpCheck.new
    instance.check_ip
  end
end

if __FILE__ == $0
  IpCheck.run!
end

# credits
# https://github.com/coverslide
