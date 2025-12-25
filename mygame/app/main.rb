# include FFI::CExt
# def boot args
#   GTK.dlopen "extension"
# end

def boot(args)
  GTK.dlopen("extension")
end


def tick args
  state = args.state

  state.button ||= {
    x: Grid.w / 2,
    y: Grid.h / 2,
    primitive_marker: :sprite,
    path: :solid,
    anchor_x: 0.5,
    anchor_y: 0.5,
    w: 200,
    h: 100,
    r: 100,
    b: 100,
    g: 100,
    a: 255
  }

  state.button.prefab ||= [
    {
      x: state.button.x,
      y: state.button.y,
      w: state.button.w,
      h: state.button.h,
      anchor_x: 0.5,
      anchor_y: 0.5,
      text: "Open File Dialog",
      primitive_marker: :label
    }
  ]

  if args.inputs.mouse.click && args.inputs.mouse.intersect_rect?(state.button)
    FileDialog.open
  end

  args.outputs << state.button
  args.outputs << state.button.prefab
end

def shutdown(args)
  FileDialog.cleanup
end
