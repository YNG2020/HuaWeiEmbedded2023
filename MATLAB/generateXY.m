function [x, y] = generateXY(cellID, nodeInCellID, args)
    nXCell = (args.spaceLimX2 - args.spaceLimX1 - args.cellX) / (args.cellX + args.cellXGap) + 1;
    xIDx = mod(cellID - 1, nXCell);
    yIDx = floor((cellID - 1) / nXCell);
    if nodeInCellID == 1 || nodeInCellID == 3
        x = xIDx * (args.cellX + args.cellXGap);
    elseif nodeInCellID == 2 || nodeInCellID == 4
        x = xIDx * (args.cellX + args.cellXGap) + args.cellX;
    end
    if nodeInCellID == 1 || nodeInCellID == 2
        y = yIDx * (args.cellY + args.cellYGap);
    elseif nodeInCellID == 3 || nodeInCellID == 4
        y = yIDx * (args.cellY + args.cellYGap) + args.cellY; 
    end
end