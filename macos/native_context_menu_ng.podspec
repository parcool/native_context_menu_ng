#
# To learn more about a Podspec see http://guides.cocoapods.org/syntax/podspec.html.
# Run `pod lib lint native_context_menu_ng.podspec` to validate before publishing.
#
Pod::Spec.new do |s|
  s.name             = 'native_context_menu_ng'
  s.version          = '0.0.1'
  s.summary          = 'native_context_menu_ng is a Flutter plugin that provides native right-click menus for Flutter applications, supporting three major desktop platforms (macOS, Windows and Linux).'
  s.description      = <<-DESC
A new Flutter project.
                       DESC
  s.homepage         = 'http://dev.venyore.com'
  s.license          = { :file => '../LICENSE' }
  s.author           = { 'parcool' => 'parcool2015@gmail.com' }

  s.source           = { :path => '.' }
  s.source_files     = 'Classes/**/*'
  s.dependency 'FlutterMacOS'

  s.platform = :osx, '10.11'
  s.pod_target_xcconfig = { 'DEFINES_MODULE' => 'YES' }
  s.swift_version = '5.0'
end
