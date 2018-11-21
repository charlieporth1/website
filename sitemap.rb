require 'rubygems'
require 'sitemap_generator'

SitemapGenerator::Sitemap.default_host = 'http://otih-oith.us.to'
SitemapGenerator::Sitemap.create do
  add '/index.html', :changefreq => 'daily', :priority => 0.9
  add '/view/otih-oith/home.html', :changefreq => 'weekly'
  add '/view/otih-oith/media.html', :changefreq => 'weekly'
  add '/view/otih-oith/apps.html', :changefreq => 'weekly'
  add '/view/otih-oith/contact.html', :changefreq => 'weekly'
  add '/view/otih-oith/biography.html', :changefreq => 'weekly'
  add '/view/otih-oith/apps/android.html', :changefreq => 'weekly'
  add '/view/otih-oith/apps/bsdfreebsdopenbsd.html', :changefreq => 'weekly'
  add '/view/otih-oith/apps/ubuntudebainkali.html', :changefreq => 'weekly'
  add '/view/otih-oith/apps/windows-phonedesktop.html', :changefreq => 'weekly'
  add '/view/otih-oith/apps/chrome-os.html', :changefreq => 'weekly'
  add '/view/otih-oith/media/twitter.html', :changefreq => 'weekly'
  add '/view/otih-oith/media/youtube.html', :changefreq => 'weekly'
  add '/view/otih-oith/media/google-plus.html', :changefreq => 'weekly'
  add '/view/otih-oith/media/github.html', :changefreq => 'weekly'
end
SitemapGenerator::Sitemap.ping_search_engines # Not needed if you use the rake tasks

