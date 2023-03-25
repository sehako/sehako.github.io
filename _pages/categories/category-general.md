---
title: "General Forum"
layout: archive
permalink: categories/generalforum
author_profile: true
sidebar_main: true
---


{% assign posts = site.categories.General %}
{% for post in posts %} {% include archive-single.html type=page.entries_layout %} {% endfor %}