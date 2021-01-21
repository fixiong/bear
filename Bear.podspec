#
#  Be sure to run `pod spec lint Bear.podspec' to ensure this is a
#  valid spec and to remove all comments including this before submitting the spec.
#
#  To learn more about Podspec attributes see https://guides.cocoapods.org/syntax/podspec.html
#  To see working Podspecs in the CocoaPods repo see https://github.com/CocoaPods/Specs/
#

Pod::Spec.new do |spec|

  spec.name         = "Bear"
  spec.version      = "0.1.0"
  spec.summary      = "A short description of Bear."
  spec.homepage     = "http://Bear"
  spec.license      = {
    :type => 'Copyright',
    :text => <<-LICENSE
    © 2013-2021 YiDian. All rights reserved.
    LICENSE
  } 
  spec.author             = { "szy" => "szy@yidian-inc.com" }
  spec.platform     = :ios, "10.0"
  spec.ios.deployment_target = "10.0"
  spec.source       = { :git => "http://Bear.git", :tag => "#{spec.version}" }
  spec.source_files  = 'include/**/*.{h,hpp,c,cpp}'
  spec.public_header_files = 'include/**/*.{h,hpp}'
# spec.private_header_files = 'include/**/*.{h,hpp}'
  
  spec.xcconfig = {
    "HEADER_SEARCH_PATHS" => ["$(PODS_TARGET_SRCROOT)/include"],
    'CLANG_CXX_LANGUAGE_STANDARD' => 'c++17',
    'EXCLUDED_ARCHS' => 'armv7',
  }

end
